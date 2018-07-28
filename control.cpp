#include "control.h"
#include<QDebug>
#include <QTimerEvent>

Control::Control()
{
    myNetTool = new Net();
    connect(myNetTool,SIGNAL(send_json(QJsonObject)),this,SLOT(handle_message(QJsonObject)));
    connect(Net::tcpSocket,SIGNAL(disconnected()),this,SLOT(tcp_disconnect()));

}


/**
 * @brief 定时器触发器
 * @param event
 */
void Control::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_nTimerID){
        handleTimeout();
    }
}
/**
 * @brief 定时器定时处理逻辑，1s执行一次
 */
void Control::handleTimeout()
{

    if(!isSimulation)
        return;

    //qDebug()<<QString::number(timer_count);
    if(Model.get_on_off()){ //处于送风
        //更新工作时间
        Model.work_time_add();
        update_work_time();

        //制冷模式下，检测当前温度小于等于目标温度，停止送风；制热模式下，检测当前温度大于等于目标温度，停止送风
        if((Model.get_work_model() == 0 && Model.get_curr_temperature() <= Model.get_idea_temperature())
            || (Model.get_work_model() == 1 && Model.get_curr_temperature() >= Model.get_idea_temperature())){
            if(stop_signal_sended == false){
                //发送停止送风的信号
                stop_wind_supply();
                emit send_result_to_status_bar("完成温控，请求停止送风");
                stop_signal_sended = true;
            }
        }
    }else{          //不送风的情况下，监测温差

        //判断是否是用户主动停止送风的，true表示允许送风，自动启动送风
        if(runable == true && ask_signal_sended == false){
            //温差监测
            if(Model.get_work_model() == 0){ //制冷模式下
                if(Model.get_idea_temperature() < Model.get_curr_temperature()){
//                    Model.set_on_off(true);
//                    update_on_off();
//                    timer_count = 1;
                    //发送送风请求
                    ask_wind_supply();
                    emit send_result_to_status_bar("检测到温差，请求送风！");
                    ask_signal_sended = true;
                }
            }else{
                if(Model.get_curr_temperature() < Model.get_idea_temperature()){
//                    Model.set_on_off(true);
//                    update_on_off();
//                    timer_count = 1;
                    //发送送风请求
                    ask_wind_supply();
                    emit send_result_to_status_bar("检测到温差，请求送风！");
                    ask_signal_sended = true;
                }
            }
        }
    }

    //进行温度模拟
    environment_simulate();
    update_current_temperature();

    //发送当前温度,每隔5秒发送一次
    qDebug()<<"计时器"+QString::number(temperature_timer) + "频率"+ QString::number(temperature_update_frequency);
    if(temperature_timer == temperature_update_frequency){
        send_state();
        temperature_timer = 0;
    }
    temperature_timer++;

    //定时时间+1
    timer_count++;
    if(timer_count > 60){
        timer_count=1;
    }

    //按钮定时，一秒内的操作，只发送最后一次的请求
    if(push_button_timer == 2){
        //发送送风请求
        push_button_timer=1;
        ask_wind_supply();
    }


}

/**
 * @brief 用户发起升温操作
 * @return
 */
int Control::idea_temperature_up(){
    Model.idea_temperature_up();
    LCDNumberIdeaTemp->display(QString::number(Model.get_idea_temperature()));
    //发送增加温度请求
    //ask_wind_supply();
    return Model.get_idea_temperature();
}
/**
 * @brief 降温操作
 * @return
 */
int Control::idea_temperature_down(){
    Model.idea_temperature_down();
    LCDNumberIdeaTemp->display(QString::number(Model.get_idea_temperature()));
    //发送降低温度请求
    //ask_wind_supply();

    return Model.get_idea_temperature();
}
/**
 * @brief 升速操作
 * @return
 */
int Control::idea_wind_speed_up(){
    if(Model.get_wind_speed() == 3)
        return Model.get_wind_speed();

    Model.wind_speed_up();
    switch (Model.get_wind_speed()) {
    case 1:
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_1.png);");
        break;
    case 2:
        timer_count = timer_count * 0.66;
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_2.png);");
        break;
    case 3:
        timer_count = timer_count * 0.5;
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_3.png);");
        break;
    default:
        break;
    }
    //发送风速变化请求
    //ask_wind_supply();
    push_button_timer = 2;
    emit send_result_to_status_bar("风速变化，发送送风请求");

    return Model.get_wind_speed();
}
/**
 * @brief 降速操作
 * @return
 */
int Control::idea_wind_speed_down(){
    if(Model.get_wind_speed() == 1)
        return Model.get_wind_speed();
    Model.wind_speed_down();
    switch (Model.get_wind_speed()) {
    case 1:
        timer_count = timer_count * (1.5);
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_1.png);");
        break;
    case 2:
        timer_count = timer_count * 2;
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_2.png);");
        break;
    case 3:
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_3.png);");
        break;
    default:
        break;
    }
    //发送风速变化请求
    //ask_wind_supply();
    push_button_timer = 2;
    emit send_result_to_status_bar("风速变化，发送送风请求");

    return Model.get_wind_speed();
}
/**
 * @brief 刷新屏幕信息
 */
void Control::update_screen(){
    LabelRoomName->setText(Model.get_user_name());
    LabelWorkTime->setText(Model.get_work_time());
    update_work_model();
    update_network_state();
    update_wind_speed();
    update_on_off();
    LCDNumberIdeaTemp->display(QString::number(Model.get_idea_temperature()));
    LCDNumberCurrTemp->display(QString::number(Model.get_curr_temperature()));
    LCDNumberPower->display(QString::number(Model.get_power()));
    LCDNumberTotal->display(QString::number(Model.get_cost()));
}
/**
 * @brief 刷新屏幕工作模式
 */
void Control::update_work_model(){
    //模式更新
    if(runable == false){
        LabelWorkState->setText("待机模式");
        return;
    }
    if(Model.get_on_off() == false){
        LabelWorkState->setText("保温模式");
        return;
    }
    if(Model.get_work_model() == 0){
        LabelWorkState->setText("制冷模式");
    }else if(Model.get_work_model() == 1){
        LabelWorkState->setText("制热模式");
    }
}

/**
 * @brief 刷新屏幕网络状态
 */
void Control::update_network_state(){
    if(request_count != 0){
        LabelNetState->setText("请求主机中...");
        return;
    }
    //网络状态更新
    switch (Model.get_net_work_state()) {
    case 0:
        LabelNetState->setText("网络连接中断");
        break;
    case 1:
        LabelNetState->setText("网络连接正常");
        break;
    case 2:
        LabelNetState->setText("网络连接中...");
        break;
    case 3:
        LabelNetState->setText("主机关机");
        break;
    default:
        break;
    }
}
/**
 * @brief 刷新屏幕风速展示
 */
void Control::update_wind_speed(){
    //更新风速
    switch (Model.get_wind_speed()) {
    case 1:
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_1.png);");
        break;
    case 2:
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_2.png);");
        break;
    case 3:
        LabelWindSpeed->setStyleSheet("border-image: url(:/Resource/fengsu_3.png);");
        break;
    default:
        break;
    }
}
/**
 * @brief 刷新是否运转送风
 */
void Control::update_on_off(){
    if(Model.get_on_off()){ //启动旋转
       timer->start(300);
       timer_count = 1;
    }else{
        timer->stop();
    }
    update_work_model();
}
/**
 * @brief 刷新工作时长
 */
void Control::update_work_time(){
    LabelWorkTime->setText(Model.get_work_time());
}

/**
 * @brief 更新并显示工作时长
 * @param time
 */
void Control::work_time_change(QTime time){

    Model.set_work_time(time);
    LabelWorkTime->setText(time.toString("hh:mm:ss"));
}
/**
 * @brief 更新网络状态
 * @param net_state
 */
void Control::net_work_change(int net_state){
    Model.set_net_work_state(net_state);
    update_network_state();
}
/**
 * @brief 更新当前温度
 * @param curr_temp
 */
void Control::curr_temperature_change(int curr_temp){
    Model.set_curr_temperature(curr_temp);
    LCDNumberCurrTemp->display(QString::number(curr_temp));
}
/**
 * @brief 更新使用电量
 * @param power
 */
void Control::power_change(double power){
    Model.set_power(power);
    LCDNumberPower->display(QString::number(power, 'g', 3));

}
/**
 * @brief 更新使用费用
 * @param cost
 */
void Control::cost_change(double cost){
    Model.set_cost(cost);
    LCDNumberTotal->display(QString::number(cost, 'g', 3));
}
/**
 * @brief 更新工作模式
 * @param work_model
 */
void Control::work_model_change(int work_model){
    //更改工作模式
    Model.set_work_model(work_model);
    //请求送风
//    Model.set_on_off(true);
//    update_on_off();
    runable = true;
    //发送送风请求
//    ask_wind_supply();
//    emit send_result_to_status_bar("向主机请求送风中");
    update_work_model();
}
void Control::update_current_temperature(){
    LCDNumberCurrTemp->display(QString::number(Model.get_curr_temperature()));
}

/**
 * @brief 更新工作送风状态
 * @param state
 */
void Control::on_off_change(bool state){
    Model.set_on_off(state);
    update_on_off();
}
/**
 * @brief 开关机切换
 */
void Control::on_off_switch(bool state){

    runable = state;
    if(runable == false){         //用户停止送风
        update_work_model();
        //发送停止送风请求
        stop_wind_supply();
        emit send_result_to_status_bar("向主机发送停止送风请求");
    }
}

/**
 * @brief 将当前状态保存到本地
 */
void Control::save_to_disk(){
    QFile file("config.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        out << Model.get_ip() << endl;
        out << Model.get_port() << endl;
        out << Model.get_cost() << endl;
        out << Model.get_curr_temperature() << endl;
        out << Model.get_idea_temperature() << endl;
        out << Model.get_land_state() << endl;
        out << Model.get_net_work_state() << endl;
        out << Model.get_on_off() << endl;
        out << Model.get_power() << endl;
        out << Model.get_user_name() << endl;
        out << Model.get_user_password() << endl;
        out << Model.get_wind_speed() << endl;
        out << Model.get_work_model() << endl;
        out << Model.get_work_time() << endl;
        file.close();
    }
}

/**
 * @brief 从本地读取ip和端口
 */
void Control::load_from_disk(){
    QFile file("config.txt");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        Model.set_ip(in.readLine());
        Model.set_port(in.readLine());
        file.close();
    }
}
/**
 * @brief 基于ip和端口，连接主机
 * @param ip
 * @param port
 * @return
 */
bool Control::connect_server(QString ip, QString port){
    //更新本地数据
    Model.set_ip(ip);
    Model.set_port(port);
    save_to_disk();
    emit send_result_to_status_bar("正在连接主机");
    /***********************根据ip和端口地址连接主机*********/
    bool ok = true;
    int dec_port=Model.get_port().toInt(&ok,10);
    Net::tcpSocket->connectToHost(Model.get_ip(), dec_port);       //连接服务器
    bool result = Net::tcpSocket->waitForConnected(1000);
    //result = true;
    if(result){ //连接成功
        Model.set_net_work_state(1);
    }else{
        emit send_result_to_status_bar("连接主机失败");
    }
    return result;
}

void Control::connect_tcp(){
    bool ok = true;
    int port=Model.get_port().toInt(&ok,10);
    Net::tcpSocket->connectToHost(Model.get_ip(), port);       //连接服务器
    bool result = Net::tcpSocket->waitForConnected();
    //result = true;
    qDebug()<<"线程启动成功";
    if(result){ //连接成功
        Model.set_net_work_state(1);
        emit send_result_to_status_bar("连接主机成功！");
        emit switch_window_signal(1);
    }else{
        emit send_result_to_status_bar("连接主机失败");
        QMessageBox message (QMessageBox::Critical,"失败","连接失败，重新连接吗？",QMessageBox::Ok|QMessageBox::Cancel,nullptr);
        if(message.exec() == QMessageBox::Ok){
           PushButtonConnect->setText("保存并连接");
           //on_pushButton_connect_clicked();
        }else{
            PushButtonConnect->setText("保存并连接");
        }
    }
}

/**
 * @brief 用户发起登录操作,发送连接请求
 * @param name
 * @param password
 * @return
 */
void Control::operation_user_land(QString name,QString password){
    Model.set_user_name(name);
    Model.set_user_password(password);
    //发送登录请求
    ask_login();
    //计时登录失败
    emit  send_result_to_status_bar("向主机发送登录请求");

//    Model.set_land_state(true);
//    emit switch_window_signal(0);
//    Model.set_work_time(QTime(0,0,0));
//    update_screen();
//    timer_count = 1;        //定时器清

}

/**
 * @brief 用户发起注销操作
 */
void Control::operation_user_out(){
    if(isSimulation){
        ask_logout();
    }
    Model.user_land_out();
    runable = false;         //用户关机状态
    emit signal_switch_on_off_button(false);    //更新用户界面按钮
    isSimulation = false;       //关闭温度模拟
    emit send_result_to_status_bar("向主机发送注销请求");
}

/**
 * @brief 接收来自主机的控制信号
 */
void Control::get_message_from_server(){
    //更新用电量，费用等
}

/**
 * @brief 环境温度模拟
 * @param level
 */
void Control::environment_simulate(){
    if(Model.get_on_off()){
        //处于送风状态
        if(Model.get_work_model() == 1){
            //制热模式策略
            switch (Model.get_wind_speed()) {
            case 1: //一级风，60s升一度
                if(timer_count % 60 == 0){
                    Model.current_temperature_add();
                }
                break;
            case 2: //二级风，40s升一度
                if(timer_count % 40 == 0){
                    Model.current_temperature_add();
                }
                break;
            case 3:
                if(timer_count % 20 == 0){
                    Model.current_temperature_add();
                }
                break;
            default:
                break;
            }
        }else{
            //制冷模式策略
            switch (Model.get_wind_speed()) {
            case 1: //一级风，60s降一度
                if(timer_count % 60 == 0){
                    Model.current_temperature_sub();
                }
                break;
            case 2: //二级风，40s降一度
                if(timer_count % 40 == 0){
                    Model.current_temperature_sub();
                }
                break;
            case 3:
                if(timer_count % 20 == 0){
                    Model.current_temperature_sub();
                }
                break;
            default:
                break;
            }
        }
    }else{  //不送风状态
        if(Model.get_work_model() == 1){
            //降温
            if(timer_count % 20 == 0){  //20s升温一度
                Model.current_temperature_sub();
            }

        }else{
            //升温策略
            if(timer_count % 20 == 0){
                Model.current_temperature_add();
            }
        }
    }
}

/**********************通信*************************/
/**
 * @brief 向主机请求送风
 */
void Control::ask_wind_supply() {
    if(!is_master_ready())
        return;
    QJsonObject json;
    json.insert("Type","AskWindSupply");
    json.insert("Room",Model.get_room_number());
    json.insert("Temperature",Model.get_idea_temperature());
    json.insert("WindSpeed",Model.get_wind_speed());
    json.insert("WindType",Model.get_work_model());
    myNetTool->send_message(json);
    request_count++;
    update_network_state();
}

/**
 * @brief 向主机请求停止送风
 */
void Control::stop_wind_supply() {
    if(!is_master_ready())
        return;
    QJsonObject json;
    json.insert("Type","StopWindSupply");
    json.insert("Room",Model.get_room_number());
    myNetTool->send_message(json);
    request_count++;
    update_network_state();
}

/**
 * @brief 向主机请求注销
 * @param room
 */
void Control::ask_logout() {
    if(!is_master_ready())
        return;
    QJsonObject json;
    json.insert("Type","AskLogout");
    json.insert("Room",Model.get_room_number());
    myNetTool->send_message(json);
}

void Control::ask_login(){
    //发送登录请求
    if(!is_master_ready())
        return;
    QJsonObject json;
    json.insert("Type","AskLogin");
    json.insert("Room",Model.get_room_number());
    json.insert("ID",Model.get_user_password());
    myNetTool->send_message(json);
}

/**
 * @brief 发送当前温度到主机
 */
void Control::send_state() {
    if(!is_master_ready())
        return;
    QJsonObject json;
    json.insert("Type","State");
    json.insert("Room",Model.get_room_number());
    json.insert("Temperature",Model.get_curr_temperature());
    myNetTool->send_message(json);
}


/**
 * @brief 周期性发消息
 */
void Control::send_temp_to_host() {
    if(Model.get_on_off()) {
        //if(timer_count%5 == 0)
        //    send_state(Model.get_user_name().toInt(),Model.get_curr_temperature());
    }
}

void Control::handle_message(QJsonObject json){
    QString type = json.value("Type").toString();
    if(type == "EnergyAndCost") {                               //更新用量信息
        power_change(json.value("Energy").toDouble());
        //qDebug()<<"energy:"<<Model.get_power();
        cost_change(json.value("Cost").toDouble());
    }
    else if(type == "ReplyForLogin") {                          //登录结果处理
        //解析是否登录成功
        int reply = json.value("Reply").toInt();
        qDebug()<<"reply="<<reply<<endl;
        if(reply == 0){ //登录成功
            qDebug()<<"WokeTemperature:"<<json.value("WorkTemperature").toInt()<<endl;
            Model.set_work_model(json.value("WorkMode").toInt());
            Model.set_idea_temperature(json.value("WorkTemperature").toInt());
            qDebug()<<"接收工作模式："<<QString::number(Model.get_work_model())<<" 理想温度："<<Model.get_idea_temperature();
            Model.set_land_state(true);
            emit switch_window_signal(0);
            Model.set_work_time(QTime(0,0,0));
            update_screen();
            timer_count = 1;        //定时器清
            isSimulation = true;    //开启温度模拟
            //request_count = 0;      //请求计数清0
            emit send_result_to_status_bar("登录成功，已从主机获取默认温度与工作模式");
        }else{  //登录失败
            emit send_infomation_window("用户名/密码错误，请重新登录");
            switch_window_signal(1);
        }
    }
    else if(type == "ReplyForState") {                             //温度状态消息ack
        if(json.value("Ack").toBool()){
            //主机处理成功
            //qDebug()<<"主机温度已受理";
            //emit send_result_to_status_bar("已同步温度到主机");
        }else{
            //qDebug()<<"主机温度受理失败";
            emit send_result_to_status_bar("主机获取当前温度失败");
        }
    }
    else if(type == "ReplyForWindSupply") {                        //送风请求ack
        ask_signal_sended = false;
        request_count--;
        update_network_state();

        if(json.value("Ack").toBool()){
            //主机处理成功
            qDebug()<<"送风请求已受理";
            emit send_result_to_status_bar("主机已受理送风请求");
            Model.set_on_off(true);
            update_on_off();
            timer_count = 1;
        }else{
            qDebug()<<"送风请求受理失败";
            Model.set_on_off(false);
            update_on_off();
            runable = false;    //强行关闭运行
            emit send_infomation_window("送风请求失败,与主机模式冲突");
        }
    }
    else if(type == "ReplyForStopWindSupply") {                     //停止送风ack
        stop_signal_sended = false;
        request_count--;
        update_network_state();
        if(json.value("Ack").toBool()){
            //主机处理成功
            qDebug()<<"停风请求受理成功";
            emit send_result_to_status_bar("主机已受理停止送风请求");
            Model.set_on_off(false);
            update_on_off();
            timer_count = 1;
        }else{
            //重发
            emit send_result_to_status_bar("主机受理停止送风请求失败，重启请求");
        }
    }
    else if(type == "PowerOn") {                                                         //主机开机
        masterState = true;
        //更改网络状态
        Model.set_net_work_state(1);
        update_network_state();
//        runable = true;
//        update_work_model();
//        emit signal_switch_on_off_button(true);
        //在用户允许送风的条件下，启动送风
        emit send_infomation_window("检测到主机开机在线，可发送请求");

    }
    else if(type == "PowerOff") {
        masterState = false;                                                             //主机关机
        //停止旋转
        on_off_change(false);
        update_on_off();
        //更改工作状态为待机模式
        runable = false;
        update_work_model();
        //更改网络状态
        Model.set_net_work_state(3);
        update_network_state();
        //更改从机键盘
        emit signal_switch_on_off_button(false);
        emit send_infomation_window("主机已关机，无法受理请求，请稍后进行操作");
    }
    else if(type == "SetFrequency"){
        int frequency = json.value("Frequency").toInt();
        temperature_update_frequency = frequency/1000;
        temperature_timer = 1;
        emit send_result_to_status_bar("已更改温度更新频率");
    }
    else{

    }
}

/**
 * @brief TCP连接中断处理
 */
void Control::tcp_disconnect(){
    Model.set_net_work_state(false);
    Model.set_land_state(false);
    operation_user_out();
    emit switch_window_signal(2);
}


/*************************getter and setter*****************************************/
void Control::setmodel(SlaveState model){
    this->Model = model;
}
SlaveState Control::get_model(){
    return Model;
}

bool Control::is_master_ready(){
    return masterState;
}

void Control::set_timer(QTimer *timer){
    this->timer = timer;
}

void Control::set_lcd_number_total(QLCDNumber * lcd_total){
    LCDNumberTotal = lcd_total;
}

void Control::set_lcd_number_power(QLCDNumber * lcd_power){
    LCDNumberPower = lcd_power;
}

void Control::set_lcd_number_curr_temp(QLCDNumber * lcd_curr_temp){
    LCDNumberCurrTemp = lcd_curr_temp;
}

void Control::set_lcd_number_idea_temp(QLCDNumber * lcd_idea_temp){
    LCDNumberIdeaTemp = lcd_idea_temp;
}

void Control::set_label_running_state(QLabel * label_running_state){
   LabelRunningState = label_running_state;
}

void Control::set_label_wind_speed(QLabel * label_wind_speed){
    LabelWindSpeed = label_wind_speed;
}

void Control::set_label_work_state(QLabel * label_work_state){
    LabelWorkState = label_work_state;
}

void Control::set_label_net_state(QLabel * label_net_state){
    LabelNetState = label_net_state;
}

void Control::set_label_work_time(QLabel * label_work_time){
    LabelWorkTime = label_work_time;
}

void Control::set_label_room_name(QLabel * label_room_name){
    LabelRoomName = label_room_name;
}

void Control::set_push_button_land(QPushButton * button){
    PushButtonLand = button;
}

void Control::set_push_button_connect(QPushButton * button){
    PushButtonConnect = button;
}



