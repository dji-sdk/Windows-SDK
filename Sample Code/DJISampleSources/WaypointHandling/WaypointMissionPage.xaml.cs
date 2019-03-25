using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using DJI.WindowsSDK;
using DJIWindowsSDKSample.ViewModels;
using Windows.Devices.Geolocation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage.Streams;
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


namespace DJIWindowsSDKSample.WaypointHandling
{
    public sealed class OperationException : Exception
    {
        public OperationException(String message, SDKError error) : base(String.Format(message))
        {
        }
    }
    public sealed partial class WaypointMissionPage : Page
    {
       
        private MapIcon aircraftMapIcon = null;
        MapElementsLayer routeLayer = new MapElementsLayer();
        MapElementsLayer waypointLayer = new MapElementsLayer();
        MapElementsLayer locationLayer = new MapElementsLayer();

        public WaypointMissionPage()
        {
            this.InitializeComponent();
            //init map layer
            WaypointMap.Layers.Add(routeLayer);
            WaypointMap.Layers.Add(waypointLayer);
            WaypointMap.Layers.Add(locationLayer);
            WaypointMissionViewModel.Instance.PropertyChanged += ViewModel_PropertyChanged;
        }

        private void ViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals(nameof(WaypointMissionViewModel.WaypointMission)))
            {
                RedrawWaypoint();
            }
            else if (e.PropertyName.Equals(nameof(WaypointMissionViewModel.AircraftLocation)))
            {
                var value = WaypointMissionViewModel.Instance.AircraftLocation;
                AircraftLocationChange(value);
            }
        }

        private void AircraftLocationChange(LocationCoordinate2D value)
        {
            if (aircraftMapIcon == null)
            {
                aircraftMapIcon = new MapIcon()
                {
                    NormalizedAnchorPoint = new Point(0.5, 0.5),
                    ZIndex = 1,
                    Image = RandomAccessStreamReference.CreateFromUri(new Uri("ms-appx:///Assets/phantom.svg")),
                };
                locationLayer.MapElements.Add(aircraftMapIcon);
            }
            aircraftMapIcon.Location = new Geopoint(new BasicGeoposition() { Latitude = value.latitude, Longitude = value.longitude });
        }

        ~WaypointMissionPage()
        {
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            DataContext = WaypointMissionViewModel.Instance;
            base.OnNavigatedTo(e);
            GetIfInSimulation();
        }

        private void GetIfInSimulation()
        {
            var isInSimulationValue = WaypointMissionViewModel.Instance.IsSimulatorStart;
            if (isInSimulationValue)
            {
                var aircraftLocaton = WaypointMissionViewModel.Instance.AircraftLocation;
                WaypointMap.Center = new Geopoint((new BasicGeoposition() { Latitude = aircraftLocaton.latitude, Longitude = aircraftLocaton.longitude }));
                AircraftLocationChange(aircraftLocaton);
            }
        }

       
        private void RedrawWaypoint()
        {
            List<BasicGeoposition> waypointPositions = new List<BasicGeoposition>();
            WaypointMission mission = WaypointMissionViewModel.Instance.WaypointMission;
            for (int i= 0; i < mission.waypoints.Count(); ++i)
            {
                if (waypointLayer.MapElements.Count == i)
                {
                    MapIcon waypointIcon = new MapIcon()
                    {
                        Image = RandomAccessStreamReference.CreateFromUri(new Uri("ms-appx:///Assets/waypoint.png")),
                        NormalizedAnchorPoint = new Point(0.5, 0.5),
                        ZIndex = 0,
                    };
                    waypointLayer.MapElements.Add(waypointIcon);
                }

                var geolocation = new BasicGeoposition() { Latitude = mission.waypoints[i].location.latitude, Longitude = mission.waypoints[i].location.longitude };
                (waypointLayer.MapElements[i] as MapIcon).Location = new Geopoint(geolocation);
                waypointPositions.Add(geolocation);
            }
            if (routeLayer.MapElements.Count == 0 && waypointPositions.Count >= 2)
            {
                var polyline = new MapPolyline
                {
                    StrokeColor = Color.FromArgb(255, 0, 255, 0),
                    Path = new Geopath(waypointPositions),
                    StrokeThickness = 2
                };
                routeLayer.MapElements.Add(polyline);
            }
            else
            {
                var waypointPolyline = routeLayer.MapElements[0] as MapPolyline;
                waypointPolyline.Path = new Geopath(waypointPositions);
            }

        }
    }
}
