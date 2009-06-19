using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Forms;
using Microsoft.Win32;

namespace Auto_Kindle
{
    class GlobalTools
    {
        private string exeLocation = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
        //Strips off the file:/ section of the return from System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase)to get the working location of an executable
        public static string GetexeLocation()
        {
            string exeLocation = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            char[] exeLocationArray;
            int exeLocationLen = 0;
            exeLocationArray = exeLocation.ToCharArray();
            exeLocation = "";
            exeLocationLen = exeLocationArray.Length;
            for (int i = 6; i < exeLocationLen; i++)
            {
                exeLocation = exeLocation + exeLocationArray[i];
            }
            return exeLocation + "\\";
        }

        public static bool FileExist(string fileIn)
        {
            if (File.Exists(fileIn))
                return true;
            else
                return false;
        }

        public static bool MoveFile(string fileIn, string fileOut)
        {
            if (File.Exists(fileIn))
            {
                if (File.Exists(fileOut))
                {
                    File.Replace(fileIn, fileOut, fileOut + ".bak");
                }
                else
                {
                    File.Move(fileIn, fileOut);
                }
                if (!File.Exists(fileOut))
                {
                    DialogResult fileMoveFailed = MessageBox.Show("Display File?", "File Could Not be Moved", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
                    if (fileMoveFailed == DialogResult.Yes)
                    {
                        Process runproc = new Process();
                        runproc.StartInfo.FileName = "explorer.exe";
                        runproc.StartInfo.Arguments = @"/select," + " \"" + fileIn + "\"";
                        runproc.Start();
                        return false;
                    }
                }
            }
            return true;
        }
    }
}
