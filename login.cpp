#include "login.h"
#include "ui_login.h"

#include "exam.h"
#include "passwdedit.h"

#include <QMessageBox>
#include <QToolButton>
#include <QUrl>
#include <QDesktopServices>
#include <QDir>
#include <QDebug>

float opacity1 = 0.0, opacity2 = 1.0;

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    init();

    connect(this,SIGNAL(close()),this,SLOT(close()));
}

Login::~Login()
{
    delete ui;
}

void Login::init()
{
    setWindowTitle(tr("登录"));

    ui->btn_edit->setStyleSheet("background-color:transparent;");
    ui->btn_regist->setStyleSheet("background-color:transparent;");
    ui->btn_login->setStyleSheet("background-color:transparent;");

    m_Drag = false;

    configWindow();

    QPalette palette;
    palette.setBrush(/*QPalette::Background*/this->backgroundRole(),
                     QBrush(QPixmap(":/images/QQ1.png")));
    this->setPalette(palette);

    timer1 = new QTimer;
    timer1->start(5);
    timer2 = new QTimer;
    connect(timer1, SIGNAL(timeout()), this, SLOT(slot_timer1()));
    connect(timer2, SIGNAL(timeout()), this, SLOT(slot_timer2()));

    QToolButton *keyBtn = new QToolButton(this);
    keyBtn->setIcon(QIcon(":/images/keyBoard.png"));
    keyBtn->setStyleSheet("background-color:transparent;");

    int x = ui->lineEdit_passwd->x();
    int y = ui->lineEdit_passwd->y();
    int width = ui->lineEdit_passwd->width();
    keyBtn->setGeometry(x+width-20, y, 20, 20);
    connect(keyBtn, SIGNAL(clicked()), this, SLOT(slot_getKeyBoard()));

    operateSql();

    ui->checkBox_rPasswd->setChecked(true);
    ui->lineEdit_passwd->setEchoMode(QLineEdit::Password);
}

void Login::setUser()
{
    user.userName.clear();
    user.userName = ui->cBox_account->currentText();
    user.passwd.clear();
    user.passwd = ui->lineEdit_passwd->text();
}

void Login::configWindow()
{
    //去掉窗口边框
    setWindowFlags(Qt::FramelessWindowHint);
    //获取界面的宽度
    int width = this->width();
    //构建最小化、最大化、关闭按钮
    QToolButton *minBtn = new QToolButton(this);
    QToolButton *closeBbtn= new QToolButton(this);
//    //获取最小化、关闭按钮图标
//    QPixmap minPix  = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
//    QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
//    //设置最小化、关闭按钮图标
//    minBtn->setIcon(minPix);
//    closeBbtn->setIcon(closePix);
    //设置最小化、关闭按钮在界面的位置
    minBtn->setGeometry(width-55,5,20,20);
    closeBbtn->setGeometry(width-25,5,20,20);
    //设置鼠标移至按钮上的提示信息
    minBtn->setToolTip(tr("最小化"));
    closeBbtn->setToolTip(tr("关闭"));
    //设置最小化、关闭按钮的样式
    minBtn->setStyleSheet("background-color:transparent;");
    closeBbtn->setStyleSheet("background-color:transparent;");
    //关联最小化、关闭按钮的槽函数
    connect(minBtn, SIGNAL(clicked()), this, SLOT(slot_minWindow()));
    connect(closeBbtn, SIGNAL(clicked()), this, SLOT(slot_closeWindow()));
}

void Login::operateSql()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user.db");
    if (!db.open()){
        qDebug() << "database open fail!";
    }else{
        qDebug() << "database open success!";
        QSqlQuery q;
        q.exec("CREATE TABLE userInfo (name VARCHAR PRIMARY KEY,passwd VARCHAR)");

        q.exec("insert into userInfo values ('admin','1')");
        q.exec("insert into userInfo values ('www','2')");
        q.exec("select * from userInfo");

        while (q.next())
        {
            QString userName = q.value(0).toString();
            ui->cBox_account->addItem(userName);
            QString passwd = q.value(1).toString();
            userPasswd.append(passwd);
            qDebug() << "userName:::"<< userName << "passwd:::" << passwd;
        }
        ui->cBox_account->setCurrentIndex(0);
        ui->lineEdit_passwd->setText(userPasswd.at(0));
    }
    db.close();
    qDebug()<<"database closed!";
}

void Login::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_Drag = true;
        m_point = e->globalPos() - this->pos();
        e->accept();
    }
}

void Login::mouseMoveEvent(QMouseEvent *e)
{
    if (m_Drag && (e->buttons() && Qt::LeftButton)) {
        move(e->globalPos() - m_point);
        e->accept();
    }
}

void Login::mouseReleaseEvent(QMouseEvent *e)
{
    m_Drag = false;
}

void Login::on_btn_login_clicked()
{
    if(ui->cBox_account->currentText().isEmpty() ||
            ui->lineEdit_passwd->text().isEmpty()){
        QMessageBox::warning(this,tr("警告"),tr("请输入用户名和密码！"));
    }else{
        int eFlag = 0;       //判断用户是否存在
        int zFlag = 0;       //判断用户名和密码是否匹配
        setUser();

        if(!db.open()){
            qDebug() << "database open fail login!";
        }else{
            QSqlQuery query;
            qDebug() << "database open success login!";
            query.exec("select * from userInfo");
            while (query.next())
            {
                QString userName = query.value(0).toString();
                QString passwd = query.value(1).toString();
                qDebug() << "login userName:::"<< userName << "passwd:::" << passwd;

                if(userName == user.userName){
                    eFlag = 1;              //用户存在
                    if(passwd == user.passwd){
                        zFlag = 1;          //用户名和密码匹配
                        Exam *e = new Exam;
                        e->show();

                        emit close();
                    }
                }
            }

            if(eFlag == 0){
                QMessageBox::information(this,tr("提示"),tr("用户不存在！"));
            }else{
                if(zFlag == 0){
                    QMessageBox::warning(this,tr("警告"),tr("用户密码错误！"));
                }
            }
        }
        db.close();
    }
}

void Login::on_btn_regist_clicked()
{
    setUser();
    if(user.userName.isEmpty() || user.passwd.isEmpty()){
        QMessageBox::information(this,tr("提示"),tr("请输入用户名和密码！"));
    }else{
        bool exitFlag = 0;       //判断用户是否存在

        if(!db.open()){
            qDebug() << "database open fail regist!";
        }else{
            QSqlQuery query;
            qDebug() << "database open success regist!";
            query.exec("select * from userInfo");
            while (query.next())
            {
                QString userName = query.value(0).toString();
                QString passwd = query.value(1).toString();
                qDebug() << "regist userName:::"<< userName << "passwd:::" << passwd;

                if(userName == user.userName){
                    exitFlag = 1;              //用户存在
                }
            }

            if(exitFlag == 0){
                query.exec(tr("insert into userInfo values ('%1','%2')")
                           .arg(user.userName).arg(user.passwd));
                qDebug()<<"regist:::"<<query.lastQuery();

                ui->cBox_account->addItem(user.userName);
                userPasswd.append(user.passwd);
                QMessageBox::information(this,tr("提示"),tr("注册成功！"));
            }else{
                QMessageBox::warning(this,tr("警告"),tr("用户已存在！"));
            }
        }
        db.close();
    }
}

void Login::on_btn_edit_clicked()
{
    if(ui->cBox_account->currentText().isEmpty() ||
            ui->lineEdit_passwd->text().isEmpty()){
        QMessageBox::information(this,tr("提示"),tr("请输入用户名和密码！"));
    }else{
        bool eFlag = 0;       //判断用户是否存在
        bool zFlag = 0;       //判断用户名和密码是否匹配
        setUser();

        if(!db.open()){
            qDebug() << "database open fail login!";
        }else{
            QSqlQuery query;
            qDebug() << "database open success login!";
            query.exec("select * from userInfo");
            while (query.next())
            {
                QString userName = query.value(0).toString();
                QString passwd = query.value(1).toString();
                qDebug() << "edit userName:::"<< userName << "passwd:::" << passwd;

                if(userName == user.userName){
                    eFlag = 1;              //用户存在
                    if(passwd == user.passwd){
                        zFlag = 1;          //用户名和密码匹配
                        passwdEdit passwd;
                        passwd.setLogin(this);
                        passwd.exec();
                    }
                }
            }

            if(eFlag == 0){
                QMessageBox::information(this,tr("提示"),tr("用户不存在！"));
            }else{
                if(zFlag == 0){
                    QMessageBox::warning(this,tr("警告"),tr("用户密码错误！"));
                }
            }
        }
        db.close();
    }
}

void Login::slot_minWindow()
{
    this->showMinimized();
}

void Login::slot_closeWindow()
{
    timer2->start(5);
}

void Login::slot_getKeyBoard()
{
    qDebug() << "key!";

//    QString m_path("D:/login/images/osk.exe");
//    int ret = curPath.compare(m_path);
//    qDebug() << "curPath:" << curPath << "ret:" << ret;
//    QString path;
//    if (ret != 0){
//        QStringList pathList = curPath.split("/");
//        path = pathList.at(0);
//        path.append("/login/images/osk.exe");
//    }else{
//        path = m_path;
//    }
    QString curPath = QApplication::applicationDirPath();
    curPath.append("/osk.exe");
    qDebug() << "curPath:" << curPath;

    QDesktopServices::openUrl(QUrl(curPath, QUrl::TolerantMode));
}

void Login::slot_timer1()
{
    if (opacity1 >= 1.0) {
        timer1->stop();
    }else{
        opacity1 += 0.01;
    }
    setWindowOpacity(opacity1);
}

void Login::slot_timer2()
{
    if (opacity2 <= 0.0) {
        timer2->stop();

        this->close();
    }else{
        opacity2 -= 0.01;
    }
    setWindowOpacity(opacity2);
}

void Login::on_cBox_account_activated(int index)
{
    ui->lineEdit_passwd->setText(userPasswd.at(index));
    qDebug() << "change cBox:" << ui->cBox_account->currentText()
             << userPasswd.at(index);
}
