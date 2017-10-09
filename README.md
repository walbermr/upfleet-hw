Primeiramente, boa noite.

O projeto utiliza pythonn 3.6.2 e as seguintes dependências:

anaconda, pyserial, h5py

Instale o anaconda adicionando às variáveis do sistema. Rode os comando para criar um ambiente e ativa-lo.

$ conda create -n env python=3.6.2 seaborn pyserial h5py
$ activate env

Faça o download do banco de dados em:

https://github.com/commaai/research

Crie uma pasta para os arquivos do dataset e atualize no arquivo de configurações config.json. Nesse arquivo também configure quais arquivos você quer que sejam enviados para o arduino, quais variáveis serão enviadas e as variáveis para a serial do arduino.

Após configurado rode o arquivo run.bat para automaticamente enviar os bancos de dados para o arduino atravéz da porta serial e criar as visualizações de dados.

Argumentos:
debug: Abilita mensagens de debug
no-serial: Desativa comunicação serial com o arduino


# variaveis (keys)

Keys starting with "app_" refer to the Applanix POS LV 220E. The X, Y, Z axes are aligned with Forward, Right, Down with respect to the car.

Keys starting with "fiber_" refer to the KVH 1775 fiber-optic gyro. The X, Y, Z axes are aligned with 45 degrees Right, 45 degrees Left, Up with respect to the car.

Keys starting with "velodyne_" refer to the HDL-32 Velodyne unit. 

| HDF5 key          | Description                                                              |
|-------------------|--------------------------------------------------------------------------|
| UN_D_cam1_ptr     |                                                                          |
| UN_D_cam2_ptr     |                                                                          |
| UN_D_lidar_ptr    |                                                                          |
| UN_D_radar_msg    |                                                                          |
| UN_D_rawgps       |                                                                          |
| UN_T_cam1_ptr     |                                                                          |
| UN_T_cam2_ptr     |                                                                          |
| UN_T_lidar_ptr    |                                                                          |
| UN_T_radar_msg    |                                                                          |
| UN_T_rawgps       |                                                                          |
| app_accel         | m/s^2, <Forward, Right, Down>                                            |
| app_heading       | deg                                                                      |
| app_pos           | <Lat (deg), Lon (deg), Alt (m)>                                          |
| app_speed         | m/s                                                                      |
| app_status        |                                                                          |
| app_v_yaw         | deg/s, yaw velocity                                                      |
| blinker           |                                                                          |
| brake             | brake_computer + brake_user                                              |
| brake_computer    | Commanded brake [0-4095]                                                 |
| brake_user        | User brake pedal depression [0-4095]                                     |
| cam1_ptr          | Camera frame index at this time                                          |
| cam2_ptr          |                                                                          |
| car_accel         | m/s^2, from derivative of wheel speed                                    |
| fiber_accel       | m/s^2                                                                    |
| fiber_compass     |                                                                          |
| fiber_compass_x   |                                                                          |
| fiber_compass_y   |                                                                          |
| fiber_compass_z   |                                                                          |
| fiber_gyro        | deg/s                                                                    |
| fiber_temperature |                                                                          |
| gas               | [0-1)                                                                    |
| gear_choice       | Selected gear. 0- park/neutral, 10- reverse, 11- gear currently changing |
| idx               |                                                                          |
| rpm               |                                                                          |
| rpm_post_torque   | post torque-converter                                                    |
| selfdrive         |                                                                          |
| speed             | m/s, from encoder after transmission, negative when gear is Revese       |
| speed_abs         | m/s, from encoder after transmission                                     |
| speed_fl          | Individual wheels speeds (m/s)                                           |
| speed_fr          |                                                                          |
| speed_rl          |                                                                          |
| speed_rr          |                                                                          |
| standstill        | Is the car stopped?                                                      |
| steering_angle    |                                                                          |
| steering_torque   | deg/s, despite the name, this is the steering angle rate                 |
| times             | seconds                                                                  |
| velodyne_gps      |                                                                          |
| velodyne_heading  |                                                                          |
| velodyne_imu      |                                                                          |