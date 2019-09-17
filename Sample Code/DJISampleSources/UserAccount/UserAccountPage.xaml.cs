using DJI.WindowsSDK;
using DJI.WindowsSDK.UserAccount;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace DJIWindowsSDKSample.UserAccount
{
    public sealed partial class UserAccountPage : Page
    {
        private UserAccountState _accountState;

        private UserAccountState accountState
        {
            get { return _accountState; }
            set
            {
                _accountState = value;
                this.accountStateTextBlock.Text = _accountState.ToString();
            }
        }

        public UserAccountPage()
        {
            this.InitializeComponent();

            DJISDKManager.Instance.UserAccountManager.UserAccountStateChanged += UserAccountManager_UserAccountStateChanged;
            accountState = DJISDKManager.Instance.UserAccountManager.UserAccountState;
        }


        ~UserAccountPage()
        {
            DJISDKManager.Instance.UserAccountManager.UserAccountStateChanged -= UserAccountManager_UserAccountStateChanged;
        }

        private void loginButton_Click(object sender, RoutedEventArgs e)
        {
            var loginWebView = DJISDKManager.Instance.UserAccountManager.CreateLoginView(false);

            if (contentGrid.Children.Count < 2)
            {
                contentGrid.Children.Add(loginWebView);
                Grid.SetColumn(loginWebView, 1);
            }
            else
            {
                contentGrid.Children[1] = loginWebView;
            }
        }

        private async void logoutButton_Click(object sender, RoutedEventArgs e)
        {
            var res = await DJISDKManager.Instance.UserAccountManager.Logout();
            var messageDialog = new MessageDialog(String.Format("User account logout: {0}", res.ToString()));
            await messageDialog.ShowAsync();
        }

        private async void UserAccountManager_UserAccountStateChanged(UserAccountState state, SDKError error)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                accountState = state;
                operationResTextBlock.Text = error.ToString();
            });
        }
    }
}
