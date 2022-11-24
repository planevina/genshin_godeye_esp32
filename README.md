# ESP32/ESP32-S3原神神之眼  

DIY制作的esp32原神神之眼代码部分  
PCB基于[小渣渣大佬的稻妻蒙德神之眼1.2板](https://oshwhub.com/Myzhazha/shen-zhi-yan-gua-jian-v1-2_esp32u)上改版   

简介
--
因自己想做动态表盘背景，esp32的内存不够开LVGL全屏缓存，故更换为S3模组  
考虑到设备作为饰品，经常带在身上，WIFI同步时间不便，增加了实时时钟ds1302模块，掉电也能走时（需接一个小纽扣电池）  
增加了小程序蓝牙控制/同步时间  
利用了esp32-s3原生的usb-cdc，可直接通过usb口刷机和监控串口  
参考了小影大佬的原神时钟设计，做成了动态表盘版（就是背后的齿轮会转动）  
给神之眼增加了呼吸效果，更像游戏中的效果  
兼容小渣渣原版的1.2PCB，可以直接刷（已提供编译好的固件），方形璃月版请自行修改屏幕初始化语句后再编译  

[PCB地址-立创](https://oshwhub.com/planevina/shen-zhi-yan-xiao-zha-zha-v1-2_esp32ucopy)  
[效果演示-哔哩哔哩](https://www.bilibili.com/video/BV1Pg411a7Sa/)  

更新
--
1.3版更新，增加了模式4--相册，小程序改版，可修改神之眼的蓝牙名称  
程序部分由于多版本太乱还没整理好，目前只上传了编译好的固件   

使用方法
--
如果不想自己编译，可直接用编译好的固件烧录(推荐..我实在是懒得回答问题)   

--
烧录工具：flash_download_tool 3.9.3  
严格按照目录内的烧录说明填写地址（坑，s3的bootloader地址与esp32不同）     
esp32版可直刷小渣渣原版1.2版本的pcb   
esp32-s3版根据圆形和方形选择自己需要刷的版本   


自己编译继续往下看↓↓   

**注意，esp32 for arduino库，必须是2.0.5版(及以上），不然蓝牙会有奇怪的问题**（都是泪）  

Arduino IDE  
--
请将*src*目录拷贝出去，并且将*main.cpp* 改为 *xxxxx.ino*文件（文件名跟目录名相同）  
使用了以下库  
* neironx/RTCLib@^1.6.2  
* moononournation/GFX Library for Arduino@^1.2.9  
* lvgl/lvgl@^8.3.2  
* mathertel/OneButton@^2.0.3  
* bitbank2/JPEGDEC@^1.2.7  

将*lv_conf_xxxxxx一串中文.h* 改成*lv_conf.h*后放到依赖目录，通常是*C:\Users\你的用户名\Documents\Arduino\libraries*，与lvgl目录同级  
(如果自己有其他项目需要用到lvgl，请先备份自己的*lv_conf.h*文件)  

Platform IO  
--
将*lv_conf_xxxxxx一串中文.h* 改成*lv_conf.h*后放到依赖目录，与lvgl目录同级  

烧录注意事项
--
按要求修改 *boardconf.h*文件，里面有修改说明  

*ESP32版(原版）*   
将烧录分区格式改为  huge_app  
其他按小渣渣立创上所写设置


*ESP32-S3版*  
如果是16mflash，则可以使用app3M_fat9M_16MB.csv分区，本程序支持一个9MB的fatfs分区，用于存放一个神之眼mjpeg文件（不要tf卡也能播放）  
开发板选择 ESP32-S3 Dev Module  
USB Mode "Hardware CDC and JTAG"  
USB Firmware MSC On Boot "Disabled"  
USB DFU On Boot "Disabled"  
Upload Mode "UART0 / Hardware CDC"  
USB CDC On Boot (如果需要调试则打开，否则禁用，如果打开了这个选项但是不连接串口，在有串口输出的地方会卡顿1秒左右）  

如果是全新的模组，第一次上电需按住BOOT按钮（或者用什么东西短接一下再上电），以进入下载模式  
（如果不这样做，电脑就会不停的提示找到usb口）  


小程序
--
小程序名叫 *planevina*    
我参考的[这个](https://gitee.com/hejinlv/WeChat-Ble-To-ESP32-Ble)写的，在此感谢  
可以自己参考重写一个专用的，比较简单  


按键操作说明
---
模式切换：长按按钮（超过600ms，但是不能按太长，3秒就会关机了 ）切换模式，一共4种模式：神之眼、时钟、自定义播放mjpeg、自定义播放图片（相册）      
属性切换：短按按钮（神之眼模式下）  
时钟模式切换：短按按钮（时钟模式下），可以切换正常时钟和快进时钟（装逼用）     
播放文件切换：短按按钮（自定义播放模式下），可依次切换*custom*目录下的*my?.mjpeg*，?的取值范围从0到254。例如默认播放 my0.mjpeg,按一下后播放my1.mjpeg(如果该文件存在)   
图片文件切换：短按按钮（图片播放模式下），可依次切换*pic*目录下的*p?.jpg*，?的取值范围从0到254。例如默认播放 p0.jpg,按一下后播放p1.jpg(如果该文件存在)

蓝牙模式关闭
--
长按按钮开机时，持续按住按钮不放，当屏幕出现*BLE Disabled.*后松开按键，即可关闭蓝牙模式（下次想打开需重新开机）  

文件结构
--
sd卡上4个目录:  
*mjpeg*目录，存放7属性神之眼文件，文件名分别是bcfhlsy（一个字母）.mjpeg  
*custom*目录，里面存放自定义视频文件，名称为 *my?.mjpeg* ，?的取值范围从0到254，用于自定义播放模式，如何压缩请b站搜索小渣渣的专栏   
*opening*目录，存放开机动画文件，名称与神之眼文件相同，开机后会随机选择一个播放   
*pic*目录，存放相册图片，可为任意大小，但建议尺寸为240x240以提高打开速度，名称为 *p?.jpg*，?的取值范围从0到254    


固件、神之眼文件和开机动画文件我已上传到网盘，地址如下：   
[开机动画、神之眼文件、固件](https://pan.baidu.com/s/1ADptwQIx8i5hoyltMYThzA)     
提取码：i8qr 


FATFS（这个感觉意义不大了）   
--
如果是16MB flash的模块，则可用mkfatfs生成一个包含blankeye文件的分区文件（项目内提供了一个包含无属性神之眼的分区文件）  
将它刷到模块的0x610000地址，这样在不插卡的时候也能播放无属性神之眼（或者其他你事先放进去的文件）  
[makefatfs的github](https://github.com/labplus-cn/mkfatfs)   
方法  
下载release版后，解压，在里面创建一个data目录，里面放入你想要集成的mjpeg文件  
（文件改名为blankeye，不要扩展名，大小不要超过8.8MB）  
运行  *./mkfatfs.exe -c data -s 0x900000 -t fatfs yan.bin*  
最后将生成的文件通过flash_download_tool刷到模块的0x610000地址即可  


功耗
--
为了省电，在播放神之眼和自定义文件时，时钟频率降低为160Mhz  
时钟模式的时钟频率提升为240Mhz  
功耗实测如下，续航按电池500mAh计算，理论值  
*ESP32-S3版*    
* 关闭蓝牙，神之眼模式：功耗0.46-0.55w，续航3.7小时
* 关闭蓝牙，时钟模式：功耗0.6w，续航3小时
* 打开蓝牙，神之眼模式：功耗0.78-0.88w，续航2.2小时
* 打开蓝牙，时钟模式：功耗0.9w，续航2小时

*ESP32版*    
* 关闭蓝牙，神之眼模式：功耗0.4-0.5w，续航4小时
* 关闭蓝牙，时钟模式：功耗0.5w，续航3.7小时 *注*
* 打开蓝牙，神之眼模式：功耗0.5-0.6w，续航3.3小时
* 打开蓝牙，时钟模式：功耗0.6w，续航3小时

*注：ESP32版没有时钟芯片，关闭蓝牙后不能同步时间，时钟功能就是摆设了*



