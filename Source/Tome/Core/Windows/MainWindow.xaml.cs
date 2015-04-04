﻿// --------------------------------------------------------------------------------------------------------------------
// <copyright file="MainWindow.xaml.cs" company="Tome">
//   Copyright (c) Tome. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Tome.Core.Windows
{
    using System;
    using System.Windows;
    using System.Windows.Input;

    using Tome.Help.Windows;
    using Tome.Model.Project;
    using Tome.Project.Windows;

    public partial class MainWindow : Window
    {
        #region Fields

        private AboutWindow aboutWindow;

        private TomeProject currentProject;

        private NewProjectWindow newProjectWindow;

        #endregion

        #region Constructors and Destructors

        public MainWindow()
        {
            this.InitializeComponent();
        }

        #endregion

        #region Methods

        private void CanExecuteClose(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CanExecuteHelp(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CanExecuteNew(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void ExecutedClose(object target, ExecutedRoutedEventArgs e)
        {
            this.Close();
        }

        private void ExecutedHelp(object target, ExecutedRoutedEventArgs e)
        {
            this.aboutWindow = this.ShowWindow(this.aboutWindow);
        }

        private void ExecutedNew(object target, ExecutedRoutedEventArgs e)
        {
            this.newProjectWindow = this.ShowWindow(this.newProjectWindow, this.OnNewProjectWindowClosed);
        }

        private void OnNewProjectWindowClosed(object sender, EventArgs e)
        {
            this.newProjectWindow.Closed -= this.OnNewProjectWindowClosed;

            try
            {
                var newProject = this.newProjectWindow.TomeProject;
                newProject.InitProject();
                this.currentProject = newProject;
            }
            catch (InvalidOperationException exception)
            {
                this.ShowErrorMessage("Error creating project", exception.Message);
            }
        }

        private void ShowErrorMessage(string title, string error)
        {
            MessageBox.Show(error, title, MessageBoxButton.OK, MessageBoxImage.Error, MessageBoxResult.OK);
        }

        private T ShowWindow<T>(T currentWindow) where T : Window, new()
        {
            return this.ShowWindow(currentWindow, null);
        }

        private T ShowWindow<T>(T currentWindow, EventHandler onClosed) where T : Window, new()
        {
            if (currentWindow == null || !currentWindow.IsLoaded)
            {
                currentWindow = new T { Owner = this, ShowInTaskbar = false };

                if (onClosed != null)
                {
                    currentWindow.Closed += onClosed;
                }
            }

            currentWindow.Show();
            currentWindow.Focus();
            return currentWindow;
        }

        #endregion
    }
}