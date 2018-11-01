using DJI.WindowsSDK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;


namespace DJIWindowsSDKSample.FPV
{
    public sealed partial class FPVPage : Page
    {

        private DJIVideoParser.Parser videoParser;
        public WriteableBitmap VideoSource;
        private byte[] decodedDataBuf;
        private object bufLock = new object();

        public FPVPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
            InitializeVideoFeedModule();

        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            UninitializeVideoFeedModule();
        }


        private void InitializeVideoFeedModule()
        {
            this.videoParser = new DJIVideoParser.Parser();
            this.videoParser.Initialize();
            this.videoParser.SetVideoDataCallack(0, 0, ReceiveDecodedData);
            if (DJISDKManager.Instance.SDKRegistrationResultCode == SDKError.NO_ERROR)
            {
                DJISDKManager.Instance.VideoFeeder.GetPrimaryVideoFeed(0).VideoDataUpdated += OnVideoPush;
            }
        }

        private void UninitializeVideoFeedModule()
        {
            if (DJISDKManager.Instance.SDKRegistrationResultCode == SDKError.NO_ERROR)
            {
                this.videoParser.SetVideoDataCallack(0, 0, null);
                DJISDKManager.Instance.VideoFeeder.GetPrimaryVideoFeed(0).VideoDataUpdated -= OnVideoPush;
            }
        }

        void OnVideoPush(VideoFeed sender, [ReadOnlyArray] ref byte[] bytes)
        {
            this.videoParser.PushVideoData(0, 0, bytes, bytes.Length);
        }

        async void ReceiveDecodedData(byte[] data, int width, int height)
        {
            lock (bufLock)
            {
                if (decodedDataBuf == null)
                {
                    decodedDataBuf = data;
                }
                else
                {
                    data.CopyTo(decodedDataBuf.AsBuffer());
                }
            }
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                if (VideoSource == null || VideoSource.PixelWidth != width || VideoSource.PixelHeight != height)
                {
                    VideoSource = new WriteableBitmap((int)width, (int)height);
                    fpvImage.Source = VideoSource;
                }

                lock (bufLock)
                {
                    decodedDataBuf.AsBuffer().CopyTo(VideoSource.PixelBuffer);
                }
                VideoSource.Invalidate();
            });
        }

    }
}
