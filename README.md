# Vision Vehicle

基于RK3588的视频采集和目标检测功能

目标监测功能使用 git@github.com:airockchip/rknn-toolkit2.git ，移植了yolov6的demo实现

本项目直接在目标设备上编译运行，暂没有交叉编译环境配置
## 开始

### 项目依赖

| 名称            | 用途                     |
|---------------|------------------------|
| librga        | 图像转码及裁剪                |
| rkdnn         | 目标监测                   |
| spdlog        | 日志                     |
| jsoncpp       | 解析网络传输的数据              |
| yaml-cpp      | 解析yml的配置文件             |
| rkmpp         | 相机输出的jpeg图像解码及h264编码   |
| OpenCV        | 基本的图像格式封装及监测结果标记       |
| eigen3        | opencv依赖               |
| rknn-toolkit2 | 模型推理                   |
| httplib       | httpserver,用于客户端发送控制参数 |

安装依赖

```shell
cd make-deps
python3 make-deps.py linux_arm  spdlog eigen3 jsoncpp yaml-cpp rkmpp OpenCV
```

##### 新系统上可能安装缺失的系统库

```bash
sudo apt install libssl-dev
sudo apt install libx11-dev
```

### 编译

```bash
mkdir build  
cd build
cmake .. -G"Unix Makefiles" -DARMLINUX=ON
cmake --build . --config Debug --target VisionVehicle
make 
make install
```

make install安装文件可能会失败，需要手动复制配置及模型文件到运行目录

```shell
cd 项目目录
cp ./apps/vision_vehicle/setting.yaml  ./build/
cp -r ./apps/vision_vehicle/model_det  ./build/
```

### 运行

```shell
# 如果提示无法打开日志文件，修改用户组
#usermod -a -G syslog 用户名
./VisionVehicle
```
PS:本项目只在orange pi5 pro测试运行