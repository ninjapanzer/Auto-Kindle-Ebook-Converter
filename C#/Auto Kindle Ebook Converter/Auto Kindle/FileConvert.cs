using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace Auto_Kindle
{
    class FileConvert
    {
        public static bool MobiIt(string HTMLFile)
        {
            fileLocation mobiFiles = new fileLocation();
            GatherFileInformation fileHandler = new GatherFileInformation(HTMLFile);
            mobiFiles = fileHandler.Getfiles();            
            if (mobiFiles.fileExt.ToLower() == "html" || mobiFiles.fileExt.ToLower() == "htm")
            {
                ProcessInformation mobiApp = new ProcessInformation();
                mobiApp.applicationString = @"app\mobigen.exe";
                mobiApp.applicationSwitch = null;
                mobiApp.applicationArgString = "\"" + mobiFiles.fileLocationDir + "\\" + mobiFiles.fileName + "." + mobiFiles.fileExt + "\"";
                ExecuteExternalProcessor mobiProcessor = new ExecuteExternalProcessor(mobiApp);
                if (!File.Exists(mobiFiles.fileLocationDir + "\\" + mobiFiles.fileName + ".mobi"))
                {
                    DialogResult fileConvertFailed = MessageBox.Show("Display Error Information?", "Conversion Failed", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
                    if (fileConvertFailed == DialogResult.Yes)
                    {
                        MessageBox.Show(mobiProcessor.GetapplicationStdOutput());
                    }
                    return false;
                }
                return true;
            }
            return false;
        }
    }
}
