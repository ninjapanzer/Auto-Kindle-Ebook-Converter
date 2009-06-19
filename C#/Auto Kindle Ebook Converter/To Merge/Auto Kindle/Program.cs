using System;
using System.Configuration;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.VisualBasic;

namespace Auto_Kindle
{
    class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            GatherFileInformation filehandler = new GatherFileInformation();

            if (args.Length == 0)
            {
                if (filehandler.ExecuteFileOpen())
                {
                    if (filehandler.ExecuteFolderSelect())
                    {
                        try
                        {
                            FileConvert.ConvertFile(filehandler);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Conversion Failed for: " + filehandler.FileInFullPath + "\n" + ex.ToString());
                        }
                    }
                }
            }
            else
            {
                ArgumentHandler arguments = new ArgumentHandler(args);
                filehandler = arguments.files;
                if (filehandler.FileOutLocation == null)
                {
                    if (Properties.Settings.Default.DefaultOutPath != null)
                    {
                        try
                        {
                            filehandler.FileOutLocation = Properties.Settings.Default.DefaultOutPath;
                            FileConvert.ConvertFile(filehandler);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Conversion Failed for: " + filehandler.FileInFullPath + "\n" + ex.ToString());
                        }
                    }
                    else
                    {

                        if (filehandler.ExecuteFolderSelect())
                        {
                            try
                            {
                                FileConvert.ConvertFile(filehandler);
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine("Conversion Failed for: " + filehandler.FileInFullPath + "\n" + ex.ToString());
                            }
                        }
                    }
                }
            }
        }

    }
}
