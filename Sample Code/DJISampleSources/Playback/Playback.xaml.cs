using DJI.WindowsSDK;
using DJI.WindowsSDK.Components;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Timers;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

/// <summary>
/// This file introduces how to download files from drone
/// </summary>
namespace DJIWindowsSDKSample.Playback
{
    /// <summary>
    /// ViewMolde for the PlaybackPage
    /// </summary>
    public sealed class TaskModel : INotifyPropertyChanged
    {
        private Windows.UI.Core.CoreDispatcher dispatcher = null;
        public List<MediaTask> runTasks = new List<MediaTask>();
        int completeCount = 0;
        string process = "0.00%";
        string count = "Count: ";
        string speed = "Speed: ";
        string sync = "Turn workmode green first";
        public long totalByte;
        public long cachedByte;
        public double MBSpeed;
        Timer timer;

        public string Process { get => process; set => process = value; }
        public string Count { get => count; set => count = value; }
        public string Speed { get => speed; set => speed = value; }
        public string Sync
        {
            get => sync; set
            {
                sync = value;
                dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
                {
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Sync"));
                });
            }
        }
        public int CompleteCount
        {
            get => completeCount;
            set
            {
                completeCount = value;
            }
        }

        public CoreDispatcher Dispatcher { get => dispatcher; set => dispatcher = value; }

        public event PropertyChangedEventHandler PropertyChanged;

        public void Reset()
        {
            StopTimer();
            totalByte = 0;
            cachedByte = 0;
            MBSpeed = 0;
            runTasks.Clear();
            completeCount = 0;
        }

        public void StartTimer()
        {
            if (timer == null)
            {
                timer = new Timer(1000);
                timer.Elapsed += Timer_Elapsed;
                timer.AutoReset = true;
            }
            if (timer.Enabled)
            {
                return;
            }
            timer.Enabled = true;
        }


        private void Timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            Process = (((double)cachedByte / (double)totalByte) * 100).ToString("0.00") + "%";
            Count = "Count: " + runTasks.Count.ToString() + "/" + CompleteCount.ToString();
            Speed = "Speed: " + MBSpeed.ToString("0.00");
            InvokeUpdateText();
        }

        void StopTimer()
        {
            if (timer != null && timer.Enabled)
            {
                timer.Stop();
                Process = " ";
                Count = "Count : ";
                Speed = "Speed: ";
                InvokeUpdateText();
            }
        }

        private void InvokeUpdateText()
        {
            dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Process"));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Count"));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Speed"));
            });
        }
    }

    /// <summary>
    /// ItemModel for the GridView render 
    /// </summary>
    public sealed class MediaItem
    {
        public MediaItem(MediaFile file)
        {
            this.file = file;
        }

        public readonly MediaFile file;
        public int Index { get => file.fileIndex; }

    }

    /// <summary>
    /// PlaybackPage with sample
    /// 1. Change work mode for download
    /// 2. Load file list from device
    /// 3. Download all file from device
    /// 4. Try sychronize current download
    /// 5. Defer current download back to waiting task
    /// 6. Cancel all download tasks
    /// </summary>
    public sealed partial class PlaybackPage : Page
    {

        public ObservableCollection<MediaItem> files = new ObservableCollection<MediaItem>();
        private CameraHandler cameraHandler = DJISDKManager.Instance.ComponentManager.GetCameraHandler(0, 0);
        private MediaTaskManager taskManager = new MediaTaskManager(0, 0);
        private TaskModel taskModel = new TaskModel();

        /// <summary>
        /// Page system life cycle
        /// </summary>
        public PlaybackPage()
        {
            this.InitializeComponent();
            taskModel.Dispatcher = Dispatcher;
            DataContext = taskModel;

        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            var current = await cameraHandler.GetCameraWorkModeAsync();
            var currMode = current.value?.value;
            ModeBtn.Background = new SolidColorBrush(currMode == CameraWorkMode.PLAYBACK || currMode == CameraWorkMode.TRANSCODE ? Color.FromArgb(128, 0, 255, 0) : Color.FromArgb(128, 255, 0, 0));
        }

        /// <summary>
        /// This is for GirdView thumbnail render 
        /// </summary>
        /// <param name="image"></param>
        private void LaunchFileDataTask(Image image)
        {
            var downloadReq = new MediaFileDownloadRequest()
            {
                index = (int)image.Tag,
                count = 1,
                offSet = 0,
                dataSize = 0,
                type = MediaRequestType.THUMBNAIL,
                subIndex = 0,
                segSubIndex = 0,
            };

            var dataTask = MediaTask.FromRequest(downloadReq);
            dataTask.OnDataReqResponse += async (task, request, data, bitSpeed) =>
            {
                using (InMemoryRandomAccessStream stream = new InMemoryRandomAccessStream())
                {
                    await stream.WriteAsync(data.AsBuffer());
                    stream.Seek(0);
                    await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, async () =>
                    {
                        var bitmap = new BitmapImage();
                        await bitmap.SetSourceAsync(stream);
                        image.Source = bitmap;
                    });
                }
            };

            taskManager.PushBack(dataTask);

        }

        private void ItemImage_Loaded(object sender, RoutedEventArgs e)
        {
            Image img = sender as Image;
            if (img != null)
            {
                LaunchFileDataTask(img);
            }
        }

        /// <summary>
        /// This is for download files whtn you click download button
        /// </summary>
        private async void DownloadAllFiles()
        {
            var enumerator = files.GetEnumerator();
            while (enumerator.MoveNext())
            {
                taskModel.CompleteCount += 1;
                taskModel.totalByte += enumerator.Current.file.fileSize;
            }
            this.DownloadSingle(taskModel.runTasks.Count);
            taskModel.StartTimer();
        }

        private async void DownloadSingle(int index)
        {
            if (index >= taskModel.CompleteCount)
            {
                taskModel.Reset();
                return;
            }
            var file = files[index].file;
            var request = new MediaFileDownloadRequest
            {
                index = file.fileIndex,
                count = 1,
                dataSize = -1,
                offSet = 0,
                segSubIndex = 0,
                subIndex = 0,
                type = MediaRequestType.ORIGIN
            };

            var task = MediaTask.FromRequest(request);
            var storageFile = await DownloadsFolder.CreateFileAsync(file.fileName, CreationCollisionOption.GenerateUniqueName);
            var stream = await storageFile.OpenAsync(Windows.Storage.FileAccessMode.ReadWrite);
            var outputStream = stream.GetOutputStreamAt(0);
            var fileWriter = new DataWriter(outputStream);
            task.OnDataReqResponse += (sender, req, data, speed) =>
            {
                Dispatcher.RunAsync(CoreDispatcherPriority.Low, async () =>
                {
                    taskModel.cachedByte += data.Length;
                    taskModel.MBSpeed = (speed / 8388608);
                    fileWriter.WriteBytes(data);
                    await fileWriter.StoreAsync();
                    await outputStream.FlushAsync();
                });
            };

            task.OnRequestTearDown += (sender, retCode, res) =>
            {
                Dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    taskModel.Sync = String.Format("DownloadMediaFile index {0} complete {1}", res?.dataReq.index, retCode);
                    this.DownloadSingle(this.taskModel.runTasks.Count);
                });
            };
            taskModel.runTasks.Add(task);
            taskManager.PushBack(task);

        }

        /// <summary>
        /// Developer should change the work mode to transcode or playback in order to enhance download speed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void ModeBtn_Click(object sender, RoutedEventArgs e)
        {
            var current = await cameraHandler.GetCameraWorkModeAsync();
            var currMode = current.value?.value;
            if (currMode != CameraWorkMode.PLAYBACK && currMode != CameraWorkMode.TRANSCODE)
            {
                var msg = new CameraWorkModeMsg
                {
                    value = CameraWorkMode.TRANSCODE
                };
                var retCode = await cameraHandler.SetCameraWorkModeAsync(msg);
                ModeBtn.Background = new SolidColorBrush(retCode == 0 ? Color.FromArgb(128, 0, 255, 0) : Color.FromArgb(128, 255, 0, 0));
            }
            else
            {
                var msg = new CameraWorkModeMsg
                {
                    value = CameraWorkMode.SHOOT_PHOTO
                };
                var retCode = await cameraHandler.SetCameraWorkModeAsync(msg);
                ModeBtn.Background = new SolidColorBrush(retCode != 0 ? Color.FromArgb(128, 0, 255, 0) : Color.FromArgb(128, 255, 0, 0));
            }

        }

        /// <summary>
        /// Developer should get file list first, in case developer gets the file index for download
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void Reload_Click(object sender, RoutedEventArgs e)
        {
            var result = await cameraHandler.GetCameraWorkModeAsync();
            if (result.value == null)
            {
                return;
            }
            var mode = result.value?.value;
            if (mode != CameraWorkMode.TRANSCODE && mode != CameraWorkMode.PLAYBACK)
            {
                return;
            }
            this.files.Clear();
            var fileListTask = MediaTask.FromRequest(new MediaFileListRequest
            {
                count = -1,
                index = 1,
                subType = MediaRequestType.ORIGIN,
                isAllList = true,
                location = MediaFileListLocation.SD_CARD,
            });
            fileListTask.OnListReqResponse += async (fileSender, files) =>
            {
                taskModel.Sync = String.Format("LaunchFileDataTask get files : {0}", files.Count);
                await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    files.ForEach(obj => this.files.Add(new MediaItem(obj)));
                });
            };
            fileListTask.OnRequestTearDown += (fileSender, retCode, response) =>
            {
                if (retCode == SDKError.NO_ERROR)
                {
                    return;
                }
                taskModel.Sync = String.Format("LaunchFileDataTask get files : {0}. Switch Mode or try again", retCode);
            };
            taskManager.PushBack(fileListTask);
        }

        /// <summary>
        /// After developer gets file index, download them in loop
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Download_Click(object sender, RoutedEventArgs e)
        {
            if (files.Count == 0)
            {
                taskModel.Sync = "Should reload before download";
                return;
            }
            if (taskModel.CompleteCount != 0)
            {
                taskModel.Sync = "Downloading is transfer, Cancel First";
                return;
            }
            DownloadAllFiles();
        }

        /// <summary>
        /// Try sychronize the current task, it should construct a same information task instance
        /// If sychronized susccess, both the original task and the sychronize task will receive every event simultineously
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void TrySync_Click(object sender, RoutedEventArgs e)
        {
            if (taskModel.runTasks.Count == 0)
            {
                taskModel.Sync = String.Format("runTaskZero");
                return;
            }
            var file = files[taskModel.runTasks.Count - 1].file;
            var request = new MediaFileDownloadRequest
            {
                index = file.fileIndex,
                count = 1,
                dataSize = -1,
                offSet = 0,
                segSubIndex = 0,
                subIndex = 0,
                type = MediaRequestType.ORIGIN
            };
            var task = MediaTask.FromRequest(request);

            task.OnDataReqForward += (task_sender, req, offset, count) =>
            {
                //taskModel.Sync = String.Format("Playback.SyncVideo OnDataReqResponse index {0} offset {1} count {2} ", req?.index, offset , count);
            };

            task.OnRequestTearDown += (task_sender, errCode, res) =>
            {
                //taskModel.Sync = String.Format("Playback.SyncVideo OnRequestTearDown errCode {0} index {1} ", errCode, res?.dataReq.index); 
            };


            var retCode = await taskManager.TrySync(task);
            if (retCode == 0)
            {
                taskModel.Sync = String.Format("{0} Success", task.Request.dataReq.First().index);
            }
            else
            {
                taskModel.Sync = String.Format("{0} Failure", task.Request.dataReq.First().index);
            }
            //taskModel.Sync = String.Format("TrySyncBtn_Click index {0} retCode {1} ", task.Request.dataReq.First().index, retCode);

        }

        /// <summary>
        /// If developer wants to interrupt current transfer but does not discard it
        /// Developer can use defer function postpone the current downloading task back to wait
        /// Example:
        /// 1. set suspend to 'true', in case after defer action the task would not restart immediatly
        /// 2. call defer function postpone the downloading TaskA 
        /// 3. pushfront some other TaskB 
        /// 4. set suspend to 'false', then the transfer will continue
        /// 5. the fisrt task of waiting queue is TaskB, the second one is TaskA
        /// 6. the taskmanager will start transfer TaskB from begin, and then transfer the TaskA from half
        /// 7. TaskA will not invoke teardown event though it has been did defer action
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Defer_Click(object sender, RoutedEventArgs e)
        {
            if (taskManager.Suspend)
            {
                taskManager.Suspend = false;
            }
            else
            {
                taskManager.Suspend = true;
                taskManager.DeferExecuteTask();
                taskModel.Sync = "Suspend & Defer";

            }
        }

        /// <summary>
        /// Cancel all download task
        /// </summary>
        /// <param name="sender"></param
        /// <param name="e"></param>
        private async void Cancel_Click(object sender, RoutedEventArgs e)
        {
            taskModel.CompleteCount = 0;
            var retCode = await taskManager.CancelAllTask();
            if (retCode == SDKError.NO_ERROR)
            {
                taskModel.Reset();
            }
            taskModel.Sync = String.Format("CancelAll Task retCode {0} ", retCode);
        }
    }
}