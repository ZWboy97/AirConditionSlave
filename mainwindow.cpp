#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slavestate.h"
#include "QMessageBox"
#include "QGraphicsScene"
#include "QGraphicsProxyWidget"
#include "QGraphicsView"
#include "qdebug.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SlaveState model;
    control.setmodel(model);
    control.set_label_net_state(ui->label_network_state);
    control.set_label_room_name(ui->label_room_name);
    control.set_label_running_state(ui->label_work_state);
    control.set_label_wind_speed(ui->label_wind_speed);
    control.set_label_work_state(ui->label_work_model);
    control.set_label_work_time(ui->label_work_time);
    control.set_lcd_number_curr_temp(ui->lcdNumber_current_temperature);
    control.set_lcd_number_idea_temp(ui->lcdNumber_idea_temperature);
    control.set_lcd_number_power(ui->lcdNumber_power);
    control.set_lcd_number_total(ui->lcdNumber_total);
    control.set_push_button_land(ui->pushButton_land);
    control.set_push_button_connect(ui->pushButton_connect);

    switch_window(2);//默认展示网络连接页面
    //旋转定时
    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeoutslot()));
    timer->start();
    count = 0;
    pixmap = QPixmap(":/Resource/fengsu.png");
    control.set_timer(timer);

    //测试展示
    control.cost_change(20.0);
    control.power_change(20.0);
    control.update_network_state();

    connect(&control,SIGNAL(switch_window_signal(int)),this,SLOT(switch_window(int)));
    connect(&control,SIGNAL(send_infomation_window(QString)),this,SLOT(display_infomation_window(QString)));
    connect(&control,SIGNAL(send_result_to_status_bar(QString)),this,SLOT(display_result_in_status_bar(QString)));
    connect(&control,SIGNAL(signal_switch_on_off_button(bool)),this,SLOT(switch_on_off_button(bool)));

    ui->pushButton_temp_up->setEnabled(false);
    ui->pushButton_temp_down->setEnabled(false);
    ui->pushButton_speed_up->setEnabled(false);
    ui->pushButton_speed_down->setEnabled(false);
    ui->pushButton_model_switch->setEnabled(false);

    ui->statusBar->showMessage(tr("从机启动！"),3000);
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief 登录按钮逻辑
 */
void MainWindow::on_pushButton_land_clicked()
{
    if(!control.is_master_ready()){      //检查主机是否受理
        ui->statusBar->showMessage(tr("主机当前不在线！"),6000);
        return;
    }
    if(ui->pushButton_land->text() == "正在登录..."){
        ui->statusBar->showMessage(tr("正在登录中，请稍后！"),6000);
        return;
    }
    user_account = ui->lineEdit_account->text();
    user_password = ui->lineEdit_password->text();
    if(user_account.isEmpty()||user_password.isEmpty()){
        QMessageBox::information(this, "错误", "用户名或密码不能为空", QMessageBox::Yes);

    }else{
        //发送登录信息
        control.operation_user_land(user_account,user_password);
        ui->pushButton_land->setText("正在登录...");
    }
}

/**
 * @brief 连接按钮点击
 */
void MainWindow::on_pushButton_connect_clicked()
{
    if(!control.is_master_ready())
        return;
    if(ui->pushButton_connect->text() == "正在连接..."){
        return;
    }
    ui->pushButton_connect->setText("正在连接...");
    QString ip = ui->lineEdit_server_ip->text();
    QString port = ui->lineEdit_server_port->text();

    bool result = control.connect_server(ip,port);

    if(result){ //连接成功
        switch_window(1);
    }else{
        QMessageBox message (QMessageBox::Critical,"失败","连接失败，重新连接吗？",QMessageBox::Ok|QMessageBox::Cancel,nullptr);
        if(message.exec() == QMessageBox::Ok){
            ui->pushButton_connect->setText("保存并连接");
            on_pushButton_connect_clicked();
        }else{
            ui->pushButton_connect->setText("保存并连接");
        }
    }
}


/**
 * @brief 主界面主窗口切换
 * @param index 窗口序号
 */
void MainWindow::switch_window(int index){
    switch (index) {
    case 0:
        if(control.get_model().get_land_state()){
            ui->stackedWidget->setCurrentIndex(0);
        }else{
            switch_window(1);
            QMessageBox::information(this,"消息","请先完成登录！","我知道了");
        }
        break;
    case 1:     //用户登录界面
        //检查是否已经成功连接
        if(control.get_model().get_net_work_state()==1){    //已连接，进入登录界面
           ui->stackedWidget->setCurrentIndex(1);
           ui->pushButton_land->setText("登录");
        }else{                                              //未连接，强制返回连接界面
            switch_window(2);
            QMessageBox::information(this,"消息","请先完成连接配置！","我知道了");
        }
        break;
    case 2:     //连接配置界面
        ui->stackedWidget->setCurrentIndex(2);
        control.load_from_disk();
        ui->lineEdit_server_ip->setText(control.get_model().get_ip());
        ui->lineEdit_server_port->setText(control.get_model().get_port());
        if(control.get_model().get_net_work_state() == 1){
            ui->pushButton_connect->setText("已连接");
            ui->pushButton_connect->setEnabled(false);
        }else{
            ui->pushButton_connect->setText("保存并连接");
            ui->pushButton_connect->setEnabled(true);
        }
        break;
    case 3:     //帮助界面
        ui->stackedWidget->setCurrentIndex(3);
        break;
    default:
        break;
    }
}

//升温
void MainWindow::on_pushButton_temp_up_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    control.idea_temperature_up();
}
//降温
void MainWindow::on_pushButton_temp_down_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    control.idea_temperature_down();
}
//升速
void MainWindow::on_pushButton_speed_up_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    control.idea_wind_speed_up();
}
//降速
void MainWindow::on_pushButton_speed_down_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    control.idea_wind_speed_down();
}
//开关切换
void MainWindow::on_pushButton_on_off_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    if(ui->pushButton_on_off->text() == "开机"){
        switch_on_off_button(true);
        control.on_off_switch(true);
    }else {
        switch_on_off_button(false);
        control.on_off_switch(false);
    }
}

void MainWindow::switch_on_off_button(bool state){
    if(state){
        ui->pushButton_on_off->setText("关机");
        ui->pushButton_temp_up->setEnabled(true);
        ui->pushButton_temp_down->setEnabled(true);
        ui->pushButton_speed_up->setEnabled(true);
        ui->pushButton_speed_down->setEnabled(true);
        ui->pushButton_model_switch->setEnabled(true);
    }else {
        ui->pushButton_on_off->setText("开机");
        ui->pushButton_temp_up->setEnabled(false);
        ui->pushButton_temp_down->setEnabled(false);
        ui->pushButton_speed_up->setEnabled(false);
        ui->pushButton_speed_down->setEnabled(false);
        ui->pushButton_model_switch->setEnabled(false);
    }
}


//旋转驱动
void MainWindow::timeoutslot(){
    if(count == 4){
        count = 0;
    }
    QMatrix leftmatrix;
    leftmatrix.rotate(90*count);
    ui->label_work_state->setPixmap(pixmap.transformed(leftmatrix,Qt::SmoothTransformation));
    count++;
}

/**
 * @brief 工作模式切换
 */
void MainWindow::on_pushButton_model_switch_clicked()
{
    if(!control.is_master_ready()){
        ui->statusBar->showMessage(tr("主机关机，请稍后重试"),6000);
        return;
    }
    if(control.get_model().get_work_model() == 0){
        control.work_model_change(1);
        ui->statusBar->showMessage(tr("切换为制热模式"),6000);
    }
    else{
        control.work_model_change(0);
        ui->statusBar->showMessage(tr("切换为制冷模式"),6000);
    }

}

/**
 * @brief 登录菜单点击
 */
void MainWindow::on_action_load_triggered()
{
    if(control.get_model().get_land_state()){
        QMessageBox::information(this,"已登录","您已经登录！","我知道了");
    }else{
        switch_window(1);
    }
}
/**
 * @brief 注销菜单点击
 */
void MainWindow::on_action_landout_triggered()
{
    if(!control.is_master_ready()){
        QMessageBox::information(this,"消息","主机当前不在线","我知道了");
        return;
    }
    if(control.get_model().get_land_state()){
        QMessageBox message (QMessageBox::Information,"注销","确认注销吗？",QMessageBox::Ok|QMessageBox::Cancel,nullptr);
        if(message.exec() == QMessageBox::Ok){
            switch_window(1);
            control.operation_user_out();
        }
    }else{
        switch_window(1);
        QMessageBox::information(this,"消息","请先登录","我知道了");
    }
}
/**
 * @brief 应用退出菜单点击
 */
void MainWindow::on_action_quit_triggered()
{
    QMessageBox message (QMessageBox::Information,"退出","确认退出系统吗？",QMessageBox::Ok|QMessageBox::Cancel,nullptr);
    if(message.exec() == QMessageBox::Ok){
        control.operation_user_out();
        exit(0);
    }
}

/**
 * @brief 配置界面菜单点击
 */
void MainWindow::on_action_network_setting_triggered()
{
    switch_window(2);
}


/**
 * @brief 返回控制面板操作
 */
void MainWindow::on_action_control_ban_triggered()
{
    switch_window(0);
}
/**
 * @brief 在帮助界面点击返回
 */
void MainWindow::on_pushButton_help_back_clicked()
{
    switch_window(0);
}
/**
 * @brief 切换到帮助界面
 */
void MainWindow::on_action_help_triggered()
{
    switch_window(3);
}

void MainWindow::display_infomation_window(QString message){
    QMessageBox::information(this,"消息",message,"我知道了");
}

/**
 * @brief 状态栏信息
 * @param result_message
 */
void MainWindow::display_result_in_status_bar(QString result_message){
    ui->statusBar->showMessage(result_message,6000);
}




