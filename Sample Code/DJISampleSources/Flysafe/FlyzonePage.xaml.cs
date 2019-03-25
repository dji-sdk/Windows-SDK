using DJI.WindowsSDK;
using DJI.WindowsSDK.FlySafe;

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Devices.Geolocation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Maps;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace DJIWindowsSDKSample.Flysafe
{

    public sealed partial class FlyzonePage : Page
    {
        private BasicGeoposition simualtionLocation = new BasicGeoposition() { Latitude = 22.6308, Longitude = 113.812 };

        public FlyzonePage()
        {
            this.InitializeComponent();
            this.FlyZoneMap.Center = new Geopoint(simualtionLocation);
            this.FlyZoneMap.ZoomLevel = 12;
        }


        private async void simulatButton_Click(object sender, RoutedEventArgs e)
        {
            String dialogMsg = "";
            do
            {
                var isInSimulatorRes = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).GetIsSimulatorStartedAsync();
                if (isInSimulatorRes.error == SDKError.NO_ERROR && isInSimulatorRes.value == null && isInSimulatorRes.value.Value.value)
                {
                    //stop simulator
                    var stopErr = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).StopSimulatorAsync();
                    if (stopErr != SDKError.NO_ERROR)
                    {
                        dialogMsg = String.Format("Failed stopping previous simulation. Error:{0}.", stopErr.ToString());
                        break;
                    }
                }

                var startErr = await DJISDKManager.Instance.ComponentManager.GetFlightControllerHandler(0, 0).StartSimulatorAsync(new SimulatorInitializationSettings() { latitude = simualtionLocation.Latitude, longitude = simualtionLocation.Longitude, satelliteCount = 15 });
                dialogMsg = startErr == SDKError.NO_ERROR ? "Success." : String.Format("Failed starting simulation. Error:{0}.", startErr.ToString());

            } while (false);

            var messageDialog = new MessageDialog(dialogMsg);
            await messageDialog.ShowAsync();
        }

        private async void showFlyzoneButton_Click(object sender, RoutedEventArgs e)
        {
            var result = await DJISDKManager.Instance.FlyZoneManager.GetFlyZoneHandler(0).GetFlyZonesInSurroundArea();

            if (result.error == SDKError.NO_ERROR)
            {
                await Window.Current.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    var areas = result.value as List<FlyZoneInformation>;
                    if (areas == null)
                    {
                        return;
                    }
                    //show areas
                    if (areas.Count > 0)
                    {
                        foreach (var area in areas)
                        {
                            var elements = MapElementBySingleFlyZone(area);
                            if (elements != null
                            && elements.Count > 0)
                            {
                                foreach (var element in elements)
                                {
                                    this.FlyZoneMap.MapElements.Add(element);
                                }
                            }
                        }
                    }
                });
            }
            else
            {
                await Window.Current.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {

                });
            }

        }

        private List<MapElement> MapElementBySingleFlyZone(FlyZoneInformation flyZone)
        {
            List<MapElement> elements = new List<MapElement>();
            if (flyZone.Type == FlyZoneType.Circle)
            {
                BasicGeoposition center = new BasicGeoposition { Latitude = flyZone.Center.latitude, Longitude = flyZone.Center.longitude };
                MapElement element = new MapPolygon
                {
                    FillColor = GetFillColor(flyZone.Category, 0),
                    Path = new Geopath(CalculateCircle(center, flyZone.Radius)),
                };
                elements.Add(element);
            }
            else if (flyZone.Type == FlyZoneType.Polygon
                && flyZone.SubFlyZones != null)
            {
                foreach (var subFlyZone in flyZone.SubFlyZones)
                {
                    if (subFlyZone.Shape == SubFlyZoneShape.Cylinder)
                    {
                        var element = MapElementBySubCircleFlyZone(subFlyZone);
                        elements.Add(element);
                    }
                    else if (subFlyZone.Shape == SubFlyZoneShape.Polygon)
                    {
                        var element = MapElementBySubPolygonFlyZone(subFlyZone);
                        elements.Add(element);
                    }
                }
            }
            return elements;
        }

        private MapElement MapElementBySubPolygonFlyZone(SubFlyZoneInformation subFlyZone)
        {
            List<BasicGeoposition> vertices = new List<BasicGeoposition>(subFlyZone.Vertices.Count);
            foreach (var item in subFlyZone.Vertices)
            {
                vertices.Add(BasicGeopositionFromZoneLocation(item));
            }
            MapElement element = new MapPolygon
            {
                FillColor = GetFillColor(subFlyZone.Category, subFlyZone.MaximumFlightHeight),
                Path = new Geopath(vertices),
            };

            return element;
        }


        private MapElement MapElementBySubCircleFlyZone(SubFlyZoneInformation subFlyZone)
        {
            MapElement element = new MapPolygon
            {
                FillColor = GetFillColor(subFlyZone.Category, subFlyZone.MaximumFlightHeight),
                Path = new Geopath(CalculateCircle(BasicGeopositionFromZoneLocation(subFlyZone.Center), subFlyZone.Radius)),
            };

            return element;
        }

        internal BasicGeoposition BasicGeopositionFromZoneLocation(LocationCoordinate2D loc)
        {
            return new BasicGeoposition
            {
                Latitude = loc.latitude,
                Longitude = loc.longitude,
            };
        }

        const double earthRadius = 6371000D;
        const double Circumference = 2D * Math.PI * earthRadius;

        public static List<BasicGeoposition> CalculateCircle(BasicGeoposition Position, double Radius)
        {
            List<BasicGeoposition> GeoPositions = new List<BasicGeoposition>();
            for (int i = 0; i <= 360; i++)
            {
                double Bearing = ToRad(i);
                double CircumferenceLatitudeCorrected = 2D * Math.PI * Math.Cos(ToRad(Position.Latitude)) * earthRadius;
                double lat1 = Circumference / 360D * Position.Latitude;
                double lon1 = CircumferenceLatitudeCorrected / 360D * Position.Longitude;
                double lat2 = lat1 + Math.Sin(Bearing) * Radius;
                double lon2 = lon1 + Math.Cos(Bearing) * Radius;
                BasicGeoposition NewBasicPosition = new BasicGeoposition();
                NewBasicPosition.Latitude = lat2 / (Circumference / 360D);
                NewBasicPosition.Longitude = lon2 / (CircumferenceLatitudeCorrected / 360D);
                GeoPositions.Add(NewBasicPosition);
            }
            return GeoPositions;
        }

        private static double ToRad(double degrees)
        {
            return degrees * (Math.PI / 180D);
        }

        private Color GetFillColor(FlyZoneCategory category, uint limitHeight)
        {
            Color color = Colors.Red;
            color.A = 75;

            if (category == FlyZoneCategory.Restricted)
            {
                //by height
                if (limitHeight <= 5)
                {
                    color = Colors.Red;
                    color.A = 75;
                }
                else if (limitHeight <= 35)
                {
                    color = Colors.Red;
                    color.A = 25;
                }
                else
                {
                    color = Colors.Black;
                    color.A = 25;
                }
            }
            else
            {
                //by category
                switch (category)
                {
                    case FlyZoneCategory.Authorizaion:
                        {
                            color = Colors.Yellow;
                            color.A = 100;
                        }
                        break;
                    case FlyZoneCategory.Warning:
                    case FlyZoneCategory.EnchanceWarning:
                        {
                            color = Colors.Green;
                            color.A = 100;
                        }
                        break;
                }
            }

            if (category == FlyZoneCategory.Unknown)
            {
                color.A = 0;
            }

            return color;
        }

    }
}
