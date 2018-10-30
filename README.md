# DJI Windows SDK

## What Is This?

The DJI Windows SDK enables you to automate your DJI Product on a PC. You can control flight, and many subsystems of the product including the camera and gimbal. Using the DJI Windows SDK, create a customized UWP (Universal Windows Platform) app to unlock the full potential of your DJI aerial platform.

## The Latest Version
The latest SDK version is Public Beta 0.1. 

## Get Started Immediately

### Prerequisites

To use DJI Windows SDK, the following environments are required:

- A PC or a laptop. 
- Windows 10.  
- Visual Studio 2017.  
- Windows 10 SDK, version 1803. 

### Supported Products

The supported DJI products include Mavic Air and Phantom 4 Pro V2. 

For Mavic Air, only WiFi connection is supported. Therefore, the PC or the laptop should have a WiFi adapter. 

For Phantom 4 Pro V2, USB connection is supported. Setup the connection with the following steps: 

  * Install DJI Assistant 2 on the PC. 
  * Ensure the remote controller has firmware version 01.00.200 or above. Use DJI GO 4 or DJI Assistant 2 to upgrade the remote controller if the firmware is old. 
  * Make sure the remote controller is powered off. 
  * Use a USB cable to connect PC with the remote controller. **CAUTION:** Use the mini-USB port of the remote controller instead of the Type A port. 
  *  Power on the remote controller. 
  *  Go to Device Manager of Windows. If the remote controller is connected successfully, you should see two devices in the list: Remote NDIS based Internet Sharing Device #1 and DJI USB Virtual COM (COM1). The number and the COM index may show differently across PC machines. 

Insert the following XML code to Package.appxmanifest of your application: 
```
<DeviceCapability Name="serialcommunication">
    <Device Id="vidpid:2ca3 001f">
	    <Function Type="name:serialPort" />
    </Device>
</DeviceCapability>
```
This can enable the permission to access to the COM port in your UWP application. This step is done in the sample code already. 
  
**CAUTION:** It is necessary to power off the remote controller first and connect it to PC after. The remote controller will determine whether to enable PC mode during booting. If the remote controller disconnects from the PC, it has to be powered off before re-connecting to the PC. 

### Generate APP Key
To enable DJI Windows SDK, developers will need to register applications with an APP key. This is also required to run the sample in this repository. Follow the steps to create the APP key: 

Go to the <a href="http://developer.dji.com/en/user/apps" target="_blank">DJI Developer Center</a>


  * Select the "Apps" tab on the left.
  * Select the "Create App" button on the right.
  * Enter the name, platform, UWP package name, category and description of the application.
  * An application activation email will be sent to complete App Key generation.
  * The App Key will appear in the user center, and can be copied and pasted into the application.


### Run Sample Code

Git clone this repository. Open DJIWindowsSDKSample.sln. Because the dependencies and the video parser has been configured in this sample, just compile and run it using x86 as the architecture. 

To start browsing all the features in the sample, you have to input the APP key. Following the previous section and use "com.wsdk.sample" as the package name to generate an APP key. 

After SDK registers successfully, you should see the full list of features. 

## SDK API Reference

[**DJI Windows SDK API Documentation**](http://developer.dji.com/api-reference/windows-api/index.html)

## FFmpeg Customization

We have forked the original FFmpeg and added customized features to provide more video frame information including the frame's width and height, frame rate number, etc. These features will help to implement video hardware decoding. 

The sample is dynamically linked with modified libraries of <a href=http://ffmpeg.org>FFmpeg</a> licensed under the <a href=http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>LGPLv2.1</a>. The source code of these FFmpeg libraries, the compilation instructions, and the LGPL v2.1 license are provided in [Github](https://github.com/dji-sdk/FFmpeg).

## Support

You can get support from dev@dji.com
