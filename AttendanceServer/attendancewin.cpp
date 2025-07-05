#include "attendancewin.h"
#include "ui_attendancewin.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QThread>
#include <opencv.hpp>
#include <QSqlError>
#include <QTimer>
#include <QFile>
#include <QDir>
AttendanceWin::AttendanceWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AttendanceWin)
{
    ui->setupUi(this);
    //qtcpServer当有客户端连会发送newconnection
    connect(&mserver, &QTcpServer::newConnection, this, &AttendanceWin::accept_client);
    mserver.listen(QHostAddress::Any,9999);//监听，启动服务器
    bsize = 0;

    //给sql模型绑定表格
    model.setTable("employee");


    //创建一个线程
    QThread *thread = new QThread();
    //把QFaceObject对象移动到thread线程中执行
    fobj.moveToThread(thread);
    //启动线程
    thread->start();
    connect(this,&AttendanceWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_faceid信号
    connect(&fobj,&QFaceObject::send_faceid,this, &AttendanceWin::recv_faceid);
    // 诊断系统状态
    QTimer::singleShot(500, [this](){
        QSqlQuery cleanQuery;
        if(cleanQuery.exec("DELETE FROM employee WHERE faceID < 0"))
        {
            int affected = cleanQuery.numRowsAffected();
            if(affected > 0)
            {
                qDebug() << "启动清理：已删除" << affected << "条无效员工记录";
            }
        }
    });
}

AttendanceWin::~AttendanceWin()
{
    delete ui;
}

//接受客户端连接
void AttendanceWin::accept_client()
{
    //获取与客户端通信的套接字
    msocket = mserver.nextPendingConnection();

    //当客户端有数据到达会发送readyRead信号
    connect(msocket,&QTcpSocket::readyRead,this,&AttendanceWin::read_data);
}

//读取客户端发送的数据
void AttendanceWin::read_data()
{
    QDataStream stream(msocket); //把套接字绑定到数据流
    stream.setVersion(QDataStream::Qt_5_14);

    if(bsize == 0){
        if(msocket->bytesAvailable()<(qint64)sizeof(bsize)) return ;
        //采集数据的长度
        stream>>bsize;
    }

    if(msocket->bytesAvailable() < bsize)//说明数据还没有发送完成，返回继续等待
    {
        return ;
    }
    QByteArray data;
    stream>>data;
    bsize = 0;
    if(data.size() == 0)//没有读取到数据
    {
        return;
    }

    //显示图片
    QPixmap mmp;
    mmp.loadFromData(data,"jpg");
    mmp = mmp.scaled(ui->picLb->size());
    ui->picLb->setPixmap(mmp);


    //识别人脸
    cv::Mat faceImage;
    std::vector<uchar> decode;
    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    faceImage = cv::imdecode(decode, cv::IMREAD_COLOR);

    //int faceid = fobj.face_query(faceImage); //消耗资源较多
    emit query(faceImage);

}

// 1. 在服务器端添加更详细的调试信息
void AttendanceWin::recv_faceid(int64_t faceid)
{
    qDebug() << "\n=== 人脸识别处理 ===";
    qDebug() << "收到的faceID:" << faceid;

    // 先查看数据库中所有的员工信息
    QSqlQuery debugQuery("SELECT employeeID, name, faceID FROM employee");
    qDebug() << "数据库中的员工：";
    while(debugQuery.next()) {
        qDebug() << "  工号:" << debugQuery.value(0).toInt()
                 << "姓名:" << debugQuery.value(1).toString()
                 << "faceID:" << debugQuery.value(2).toLongLong();
    }

    QString sdmsg;

    if(faceid < 0)
    {
        qDebug() << "结果：识别失败（相似度不足）";
        sdmsg = QString("{\"employeeID\":\"\",\"name\":\"未识别\",\"department\":\"\",\"time\":\"%1\",\"status\":\"failed\"}")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        msocket->write(sdmsg.toUtf8());
        return;
    }

    // 查询数据库
    model.setFilter(QString("faceID=%1").arg(faceid));
    model.select();

    qDebug() << "查询条件: faceID=" << faceid;
    qDebug() << "查询结果行数:" << model.rowCount();

    if(model.rowCount() == 1)
    {
        QSqlRecord record = model.record(0);
        QString employeeID = record.value("employeeID").toString();
        QString name = record.value("name").toString();

        qDebug() << "结果：识别成功 - 工号:" << employeeID << "姓名:" << name;

        sdmsg = QString("{\"employeeID\":\"%1\",\"name\":\"%2\",\"department\":\" 电子信息工程\",\"time\":\"%3\",\"status\":\"success\"}")
                .arg(employeeID)
                .arg(name)
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        msocket->write(sdmsg.toUtf8());

        // 记录考勤
        QSqlQuery query;
        QString insertSql = QString("INSERT INTO attendance (employeeID, attendaceTime) VALUES (%1, '%2')")
                           .arg(record.value("employeeID").toInt())
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        if(!query.exec(insertSql))
        {
            qDebug() << "考勤记录插入失败：" << query.lastError().text();
        }
        else
        {
            qDebug() << "考勤记录插入成功";
        }
    }
    else
    {
        qDebug() << "结果：未注册（数据库中没有faceID=" << faceid << "的记录）";
        sdmsg = QString("{\"employeeID\":\"\",\"name\":\"未注册\",\"department\":\"\",\"time\":\"%1\",\"status\":\"unregistered\"}")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        msocket->write(sdmsg.toUtf8());
    }
    qDebug() << "===================\n";
}
