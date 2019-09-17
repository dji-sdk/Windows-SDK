using DJI.WindowsSDK;
using DJIUWPSample.Commands;
using DJIUWPSample.ViewModels;

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Popups;

namespace DJIWindowsSDKSample.ViewModels
{
    class ComponentViewModel : ViewModelBase
    {
        public String AircraftSetName { set; get; }
        String _aircraftGetName = "";
        public String AircraftGetName
        {
            get
            {
                return _aircraftGetName;
            }
            set
            {
                _aircraftGetName = value;
                OnPropertyChanged("AircraftGetName");
            }
        }
        Velocity3D _aircraftVelocity3D;
        public Velocity3D AircraftVelocity
        {
            get
            {
                return _aircraftVelocity3D;
            }
            set
            {
                _aircraftVelocity3D = value;
                OnPropertyChanged("AircraftVelocityXString");
                OnPropertyChanged("AircraftVelocityYString");
                OnPropertyChanged("AircraftVelocityZString");
            }
        }
        public String AircraftVelocityXString
        {
            get { return _aircraftVelocity3D.x.ToString() + " m/s"; }
        }
        public String AircraftVelocityYString
        {
            get { return _aircraftVelocity3D.y.ToString() + " m/s"; }
        }
        public String AircraftVelocityZString
        {
            get { return _aircraftVelocity3D.z.ToString() + " m/s"; }
        }

        public ICommand _registerVelocityChangedObserver;
        public ICommand RegisterVelocityChangedObserver
        {
            get
            {
                if (_registerVelocityChangedObserver == null)
                {
                    _registerVelocityChangedObserver = new RelayCommand(delegate ()
                    {
                        DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).VelocityChanged += ComponentHandingPage_VelocityChanged;
                    }, delegate () { return true; });
                }
                return _registerVelocityChangedObserver;
            }
        }
        private async void ComponentHandingPage_VelocityChanged(object sender, Velocity3D? value)
        {
            await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                AircraftVelocity = value.Value;
            });
        }
        public ICommand _setAircraftName;
        public ICommand SetAircraftName
        {
            get
            {
                if (_setAircraftName == null)
                {
                    _setAircraftName = new RelayCommand(async delegate()
                    {
                        String message = "";
                        do
                        {
                            var toSet = AircraftSetName;
                            if (toSet==null || toSet.Length == 0)
                            {
                                message = "Input your name first!";
                                break;
                            }
                            var res = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).SetAircraftNameAsync(new StringMsg { value = toSet });
                            message = String.Format("Set aircraft's name: {0}", res.ToString());
                        } while (false);
                        var messageDialog = new MessageDialog(message);
                        await messageDialog.ShowAsync();
                    }, delegate () { return true; });
                }
                return _setAircraftName;
            }
        }
        public ICommand _getAircraftName;
        public ICommand GetAircraftName
        {
            get
            {
                if (_getAircraftName == null)
                {
                    _getAircraftName = new RelayCommand(async delegate ()
                    {
                        String message = "";
                        do
                        {
                            var res = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).GetAircraftNameAsync();
                            if (res.error != SDKError.NO_ERROR)
                            {
                                message = String.Format("Get aircraft's name: {0}", res.error.ToString());
                                break;
                            }
                            if (res.value != null)
                            {
                                AircraftGetName = res.value.Value.value;
                            }
                            message = String.Format("Get aircraft's name Success.");
                        } while (false);

                        var messageDialog = new MessageDialog(message);
                        await messageDialog.ShowAsync();
                    }, delegate () { return true; });
                }
                return _getAircraftName;
            }
        }
        public ICommand _startTakeoff;
        public ICommand StartTakeoff
        {
            get
            {
                if (_startTakeoff == null)
                {
                    _startTakeoff = new RelayCommand(async delegate ()
                    {
                        var res = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).StartTakeoffAsync();
                        var messageDialog = new MessageDialog(String.Format("Start send takeoff command: {0}", res.ToString()));
                        await messageDialog.ShowAsync();
                    }, delegate () { return true; });
                }
                return _startTakeoff;
            }
        }
    }
}
