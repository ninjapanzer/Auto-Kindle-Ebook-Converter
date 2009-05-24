using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using Auto_Kindle;
using Global_Tools;

namespace Auto_Kindle_Config
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        bool Saved = false;

        public Window1()
        {
            InitializeComponent();
            ReadConfigFile();
        }
        ~Window1()
        {

        }

        public void ReadConfigFile()
        {
            if (GlobalTools.FileExist("Config.pnz"))
            {
                string configInput;
                string[] configElement;
                TextReader ReadText = new StreamReader("Config.pnz");
                while ((configInput = ReadText.ReadLine()) != null)
                {
                    //configInput = ReadText.ReadLine();
                    configElement = configInput.Split(':');
                    if (configElement[0] == "OutputLoc")
                    {
                        if (configElement[1] == "UserDefine")
                        {
                            UserDefineOutputLocChk.IsChecked = true;
                            OutputLocTxtBox.Text = ReadText.ReadLine();
                        }
                        else if (configElement[1] == "InputLocation")
                        {
                            OutputDefaultChk.IsChecked = true;
                        }
                        else if (configElement[1] == "Default")
                        {
                            DefaultLocDisableChk.IsChecked = true;
                        }
                    }
                    if (configElement[0] == "PDFImage")
                    {
                        if (configElement[1] == "true")
                        {
                            PDFImageProcessing.IsChecked = true;
                        }
                        else
                        {
                            PDFImageProcessing.IsChecked = false;
                        }
                    }
                }
            }
        }


        public void WriteOutputFile()
        {
            TextWriter WriteConfig = new StreamWriter("Config.pnz");
            if (UserDefineOutputLocChk.IsChecked == true)
            {
                WriteConfig.WriteLine("OutputLoc:UserDefine");
                WriteConfig.WriteLine(OutputLocTxtBox.Text);
            }
            else if (OutputDefaultChk.IsChecked == true)
            {
                WriteConfig.WriteLine("OutputLoc:InputLocation");
            }
            else if (DefaultLocDisableChk.IsChecked == true)
            {
                WriteConfig.WriteLine("OutputLoc:Default");
            }
            if (PDFImageProcessing.IsChecked == true)
            {
                WriteConfig.WriteLine("PDFImage:true");
            }
            else if (PDFImageProcessing.IsChecked == false)
            {
                WriteConfig.WriteLine("PDFImage:false");
            }
            this.Saved = true;
            //WriteConfig.WriteLine(DateTime.Now);
            WriteConfig.Close();
        }

        private void OutputSelect_Click(object sender, RoutedEventArgs e)
        {
            //fileLocation fileLoc;
            GatherFileInformation filehandler = new GatherFileInformation();
            OutputLocTxtBox.Text = filehandler.ExecuteFolderSelect();
        }

        private void OutputDefaultChk_Checked(object sender, RoutedEventArgs e)
        {
            if (OutputDefaultChk.IsChecked == true)
            {
                DefaultLocDisableChk.IsChecked = false;
                UserDefineOutputLocChk.IsChecked = false;
                OutputLocTxtBox.Text = "";
            }
            else if (OutputDefaultChk.IsChecked == false)
                if (OutputDefaultChk.IsChecked == false && UserDefineOutputLocChk.IsChecked == false)
                {
                    DefaultLocDisableChk.IsChecked = true;
                }
            {
                //DefaultLocDisableChk.IsChecked = true;
            }
        }

        private void OutputLocTxtBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (UserDefineOutputLocChk.IsChecked == false && OutputLocTxtBox.Text != "")
            {
                UserDefineOutputLocChk.IsChecked = true;
            }
                if (UserDefineOutputLocChk.IsChecked == true)
                {
                    if (OutputLocTxtBox.Text == "")
                    {
                        UserDefineOutputLocChk.IsChecked = false;
                    }
            }
        }

        private void DefaultLocDisableChk_Checked(object sender, RoutedEventArgs e)
        {
            if (DefaultLocDisableChk.IsChecked == true)
            {
                OutputDefaultChk.IsChecked = false;
                UserDefineOutputLocChk.IsChecked = false;
                OutputLocTxtBox.Text = "";
            }
        }
        private void DefaultLocDisableChk_Unchecked(object sender, RoutedEventArgs e)
        {
            if (OutputDefaultChk.IsChecked == false && UserDefineOutputLocChk.IsChecked == false)
            {
                DefaultLocDisableChk.IsChecked = true;
            }
        }

        private void UserDefineOutputLocChk_Checked(object sender, RoutedEventArgs e)
        {
            if (UserDefineOutputLocChk.IsChecked == true)
            {
                DefaultLocDisableChk.IsChecked = false;
                OutputDefaultChk.IsChecked = false;
            }
        }

        private void UserDefineOutputLocChk_Unchecked(object sender, RoutedEventArgs e)
        {
            if (OutputDefaultChk.IsChecked == false && UserDefineOutputLocChk.IsChecked == false)
            {
                DefaultLocDisableChk.IsChecked = true;
            }
            //DefaultLocDisableChk.IsChecked = true;
        }

        private void SaveBtn_Click(object sender, RoutedEventArgs e)
        {
            WriteOutputFile();
        }

        private void ResetBtn_Click(object sender, RoutedEventArgs e)
        {
            DefaultLocDisableChk.IsChecked = true;
            PDFImageProcessing.IsChecked = true;
            WriteOutputFile();
        }

    }
}
