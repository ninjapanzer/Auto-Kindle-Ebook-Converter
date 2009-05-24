using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.VisualBasic;
using Global_Tools;

namespace Auto_Kindle
{
    class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            GlobalTools.CheckPathInRegRestart("thing");
            GatherFileInformation filehandler = new GatherFileInformation();
            ProcessInformation mobi2mobiApp = new ProcessInformation();
            ProcessInformation clitApp = new ProcessInformation();
            ProcessInformation pdbApp = new ProcessInformation();
            ProcessInformation chmApp = new ProcessInformation();
            ProcessInformation pdfApp = new ProcessInformation();
            string workingDir = GlobalTools.GetexeLocation();
            string tempFolder = workingDir + @"Temp\";
            mobi2mobiApp.applicationString = @"app\mobi2mobi.exe";
            mobi2mobiApp.applicationString = null;
            clitApp.applicationString = @"app\clit.exe";
            clitApp.applicationSwitch = @"-d";
            pdbApp.applicationString = @"app\abcpalm.exe";
            pdbApp.applicationSwitch = null;
            chmApp.applicationString = @"app\chmdecoder.exe";
            chmApp.applicationSwitch = @"-menu";
            pdfApp.applicationString = @"app\pdftohtml.exe";
            pdfApp.applicationSwitch = @"-noframes -c";
            fileLocation fileLocs;
            if (args.Length == 0)
            {
                filehandler.ExecuteFileOpen();
                filehandler.ExecuteFolderSelect();
                fileLocs = filehandler.files;
                if (fileLocs.fileExt.ToLower() == "htm")
                {
                    fileLocs.fileTemp = fileLocs.fileExt;
                    fileLocs.fileExt = "html";
                }
                
                switch (fileLocs.fileExt.ToLower())
                {
                    case "html":
                        if (fileLocs.fileTemp == "htm")
                        {
                            fileLocs.fileExt = fileLocs.fileTemp;
                        }
                        if (FileConvert.MobiIt(fileLocs.fileIn))
                        {
                            GlobalTools.MoveFile(fileLocs.fileLocationDir + "\\" + fileLocs.fileName + ".mobi", fileLocs.fileOut + "\\" + fileLocs.fileName + ".mobi");
                        }
                        //mobiApp.applicationArgString = fileLocs.fileIn_Escape;
                        //ExecuteExternalProcessor mobiProcessor = new ExecuteExternalProcessor(mobiApp);
                        break;
                    case "mobi":
                        string newTitle = Interaction.InputBox("Enter a New Title for This Book", "MetaData Manipulation", fileLocs.fileName, 100, 100);
                        mobi2mobiApp.applicationArgString = @"--title " + "\"" + newTitle + "\" " + fileLocs.fileIn_Escape + @" --outfile " + fileLocs.fileIn_Escape;
                        ExecuteExternalProcessor mobi2mobiProcessor = new ExecuteExternalProcessor(mobi2mobiApp);
                        break;
                    case "pdb":
                        pdbApp.applicationArgString = fileLocs.fileIn_Escape + " \"" +  tempFolder + fileLocs.fileName + ".html\" \"1\"";
                        //MessageBox.Show(pdbApp.applicationArgString);
                        ExecuteExternalProcessor pdbProcessor = new ExecuteExternalProcessor(pdbApp);
                        if (FileConvert.MobiIt(tempFolder + fileLocs.fileName + ".html"))
                        {
                            GlobalTools.MoveFile(fileLocs.fileLocationDir + "\\" + fileLocs.fileName + ".mobi", fileLocs.fileOut + "\\" + fileLocs.fileName + ".mobi");
                        }
                        break;
                    case "lit":
                        clitApp.applicationArgString = fileLocs.fileIn_Escape +" \"" + tempFolder + "\"";
                        ExecuteExternalProcessor litProcessor = new ExecuteExternalProcessor(clitApp);
                        if (GlobalTools.FileExist(tempFolder + fileLocs.fileName + ".html"))
                        {
                            if (FileConvert.MobiIt(tempFolder + fileLocs.fileName + ".html"))
                            {
                                GlobalTools.MoveFile(fileLocs.fileLocationDir + "\\" + fileLocs.fileName + ".mobi", fileLocs.fileOut + "\\" + fileLocs.fileName + ".mobi");
                            }
                        }
                        else
                        {
                            MessageBox.Show(litProcessor.GetapplicationErrorOutput(), "Lit Convert Failed");
                        }
                        break;
                    case "chm":
                        if (fileLocs.fileIn_Escape == "1")
                        {
                            chmApp.applicationArgString = fileLocs.fileIn_Escape + " \"" + tempFolder + "\"";
                            ExecuteExternalProcessor chmProcessor = new ExecuteExternalProcessor(chmApp);
                        }
                        break;
                    case "pdf":
                        pdfApp.applicationArgString = fileLocs.fileIn_Escape + " \"";
                        ExecuteExternalProcessor pdfProcessor = new ExecuteExternalProcessor(pdfApp);
                        if (FileConvert.MobiIt(tempFolder + fileLocs.fileName + "s.html"))
                        {
                            GlobalTools.MoveFile(tempFolder + fileLocs.fileName + "s.mobi", fileLocs.fileOut + "\\" + fileLocs.fileName + ".mobi");
                        }
                        break;
                    default:
                        MessageBox.Show("File type not supported", "Not Supported");
                        break;
                }
            }
            else
            {
                ArgumentHandler arguments = new ArgumentHandler(args);
                fileLocs = arguments.files;
                if (fileLocs.fileOut == null)
                {
                    fileLocs.fileOut = filehandler.ExecuteFolderSelect();
                }
            }

        }

    }
}
