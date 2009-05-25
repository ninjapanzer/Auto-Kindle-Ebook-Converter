using System;
using System.Collections.Generic;
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
using System.Net;
using System.IO;
using Global_Tools;

namespace AutoKindle_Updater
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();
        }

        private void textBox1_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            Ftp ftpaccess = new Ftp(GlobalTools.GetexeLocation(), "simple.pnz", Ftp.dnsResolve("ftp.technosamurai.net"), "public@technosamurai.net", "public");
            StatusTxt.AppendText("Creating Missing Folders\n");
            Queue<string> fileNameList = ftpaccess.getFileListQueue();
            ftpaccess.ftpCred.filePath = GlobalTools.GetexeLocation();
            foreach (string fileName in fileNameList)
            {
                //if (GlobalTools.FileExist(GlobalTools.GetexeLocation() + fileName))
                //{
                ftpaccess.ftpCred.fileName = fileName;
                StatusTxt.AppendText("Getting " + fileName + "......");
                ftpaccess.getFileFtp();
                StatusTxt.AppendText("Finished\n");
                //}
            }
        }
    }
}
