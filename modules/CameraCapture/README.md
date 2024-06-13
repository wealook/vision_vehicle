# 采集模块

采集模块目前已经支持的相机包括：
    blt（Melexis TOF）
    realsense (intel)
    webcam (opencv video capture)
    fake (file read/write)

## 文件相机说明

之所以开发文件fake相机，是为了方便调试和复现问题，也便于进行单元测试。

### 录制格式

    目前的文件相机比较简单，是将一帧图片保存成两个png文件的方式，一个是RGB/Gray，另外一个是16位的Depth。保存的代码在SimpleHandTracking.cpp

    `````
   	if (isRecording_ && !cachePath_.empty())
	{
		std::stringstream grayFilePath;
		grayFilePath << cachePath_ << "/gray_" << index_ << ".png";
		cv::imwrite(grayFilePath.str(), gray);

		std::stringstream depthPath;
		depthPath << cachePath_ << "/depth_" << index_ << ".png";
		cv::imwrite(depthPath.str(), depth);

		index_++;
	}

    `````

### Demo程序的录制和回放

    Demo程序的文件菜单，开始并录制，会保存到 根目录的`samples/outputimages`，录制好的文件请自己打包备份起来，再次录制会直接覆盖。
    Demo程序的测试菜单会从刚刚的目录回放录制的文件。

    创建Capture的时候如果使用fake相机，第二个参数是路径。
    