using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;


namespace DJIUWPSample.ViewModels
{
    /// <summary>
    /// Base implementation of the <see cref="INotifyPropertyChanged"/> interface.
    /// </summary>
    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        /// <summary>
        /// Occurs immediately after a property of this instance has changed.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Raises the <see cref="PropertyChanged"/> event.
        /// </summary>
        protected void OnPropertyChanged([CallerMemberName] string changedPropertyName = "")
        {
            // The OnPropertyChanged is not virtual itself because of the [CallerMemberName] attribute which in case overridden should be put in every override - quite error-prone.
            this.PropertyChangedOverride(changedPropertyName);

            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(changedPropertyName));
        }

        /// <summary>
        /// Provides an entry point for inheritors to provide additional logic over the PropertyChanged routine.
        /// </summary>
        protected virtual void PropertyChangedOverride(string changedPropertyName)
        {
        }
    }
}
