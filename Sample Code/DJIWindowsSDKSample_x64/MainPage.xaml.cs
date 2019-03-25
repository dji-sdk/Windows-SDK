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
using Windows.UI.Xaml.Navigation;


using DJI.WindowsSDK;

// https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x804 上介绍了“空白页”项模板

namespace DJIWindowsSDKSample
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private struct SDKModuleSampleItems
        {
            public String header;
            public List<KeyValuePair<String, Type>> items;
        }

        private List<SDKModuleSampleItems> navigationModules = new List<SDKModuleSampleItems>
        {
            new SDKModuleSampleItems() {
                header = "Activation", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Activating DJIWindowsSDK", typeof(DJISDKInitializing.ActivatingPage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "FPV", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("FPV", typeof(FPV.FPVPage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "Component Handling", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Component Handling", typeof(ComponentHandling.ComponentHandingPage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "Waypoint", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Using Simulator", typeof(WaypointHandling.SimulatorPage)),
                    new KeyValuePair<string, Type>("Waypoint Mission", typeof(WaypointHandling.WaypointMissionPage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "Account", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Account Management", typeof(UserAccount.UserAccountPage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "Flysafe", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Flyzone", typeof(Flysafe.FlyzonePage)),
                },
            },
            new SDKModuleSampleItems() {
                header = "Playback", items = new List<KeyValuePair<String, Type>>()
                {
                    new KeyValuePair<string, Type>("Playback", typeof(Playback.PlaybackPage)),
                },
            },

        };

        public MainPage()
        {
            this.InitializeComponent();
            var module = navigationModules[0];
            NavView.MenuItems.Add(new NavigationViewItemHeader() { Content = module.header });
            foreach (var item in module.items)
            {
                NavView.MenuItems.Add(item.Key);
            }
        }

        private void NavView_ItemInvoked(NavigationView sender, NavigationViewItemInvokedEventArgs args)
        {
            String invokedName = args.InvokedItem as String;
            foreach (var module in navigationModules)
            {
                foreach (var item in module.items)
                {
                    if (invokedName == item.Key)
                    {
                        if (ContentFrame.SourcePageType != item.Value)
                        {
                            ContentFrame.Navigate(item.Value);
                        }
                        return;
                    }
                }
            }
        }

        private void NavView_Loaded(object sender, RoutedEventArgs e)
        {
            DJISDKManager.Instance.SDKRegistrationStateChanged += Instance_SDKRegistrationEvent;
        }

        private async void Instance_SDKRegistrationEvent(SDKRegistrationState state, SDKError resultCode)
        {
            if (resultCode == SDKError.NO_ERROR)
            {
                await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    for (int i = 1; i < navigationModules.Count;++i)
                    {
                        var module = navigationModules[i];
                        NavView.MenuItems.Add(new NavigationViewItemHeader() { Content = module.header });
                        foreach (var item in module.items)
                        {
                            NavView.MenuItems.Add(item.Key);
                        }
                    }
                });
            }
        }
    }
}
