using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Windows.Forms;
using Microsoft.VisualBasic;
using System.Xml;

namespace Auto_Kindle
{
    class FileConvert
    {
        private static ProcessInformation mobi2mobiApp;
        private static ProcessInformation clitApp;
        private static ProcessInformation pdbApp;
        private static ProcessInformation chmApp;
        private static ProcessInformation pdfApp;
        private static string workingDir;
        private static string tempFolder;

        static FileConvert()
        {
            mobi2mobiApp = new ProcessInformation();
            clitApp = new ProcessInformation();
            pdbApp = new ProcessInformation();
            chmApp = new ProcessInformation();
            pdfApp = new ProcessInformation();

            mobi2mobiApp.applicationString = Properties.Settings.Default.mobi2mobiAppPath;
            mobi2mobiApp.applicationSwitch = Properties.Settings.Default.mobi2mobiAppArgs;
            clitApp.applicationString = Properties.Settings.Default.clitAppPath;
            clitApp.applicationSwitch = Properties.Settings.Default.clitAppArgs;
            pdbApp.applicationString = Properties.Settings.Default.pdbAppPath;
            pdbApp.applicationSwitch = Properties.Settings.Default.pdfAppArgs;
            chmApp.applicationString = Properties.Settings.Default.chmAppPath;
            chmApp.applicationSwitch = Properties.Settings.Default.chmAppArgs;
            pdfApp.applicationString = Properties.Settings.Default.pdfAppPath;
            pdfApp.applicationSwitch = Properties.Settings.Default.pdbAppArgs;

            workingDir = GlobalTools.GetexeLocation();
            tempFolder = Properties.Settings.Default.TempPath;
        }
        private static bool MobiIt(string InputFile)
        {
            GatherFileInformation fileHandler = new GatherFileInformation(InputFile);
            if (fileHandler.FileInExtension.ToLower() == ".opf")
            {
                CleanOpfFile(fileHandler.FileInFullPath);
            }
            ProcessInformation mobiApp = new ProcessInformation();
            mobiApp.applicationString = Properties.Settings.Default.MobigenPath;
            mobiApp.applicationSwitch = null;
            mobiApp.applicationArgString = fileHandler.FileInEscaped;
            ExecuteExternalProcessor mobiProcessor = new ExecuteExternalProcessor(mobiApp);
            if (!File.Exists(fileHandler.FileInLocation + "\\" + fileHandler.FileInNameNoExtension + ".mobi"))
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
        private static void CleanOpfFile(string FilePath)
        {
            try
            {
                XmlDocument doc = new XmlDocument();
                doc.Load(FilePath);
                XmlNamespaceManager nsmgr = new XmlNamespaceManager(doc.NameTable);
                nsmgr.AddNamespace("dc", "http://purl.org/dc/elements/1.0/");
                nsmgr.AddNamespace("oebpackage", "http://openebook.org/namespaces/oeb-package/1.0/");

                if (doc.SelectSingleNode("//dc:Language", nsmgr) == null)
                {
                    XmlElement metdataElement = (XmlElement)doc.DocumentElement.SelectSingleNode("//oebpackage:dc-metadata", nsmgr);
                    XmlElement languageElement = doc.CreateElement("dc", "Language", nsmgr.LookupNamespace("dc"));
                    languageElement.InnerText = "en-us";
                    metdataElement.AppendChild(languageElement);
                    using (XmlTextWriter xtw = new XmlTextWriter(FilePath, Encoding.Default))
                    {
                        doc.WriteContentTo(xtw);
                        xtw.Flush();
                        xtw.Close();
                    }
                }
            }
            catch
            { 
            }
        }
        public static void ConvertFile(GatherFileInformation filehandler)
        {
            switch (filehandler.FileInExtension.ToLower())
            {
                case ".htm":
                case ".html":
                    if (FileConvert.MobiIt(filehandler.FileInName))
                    {
                        GlobalTools.MoveFile(filehandler.FileInLocation + "\\" + filehandler.FileInNameNoExtension + ".mobi", filehandler.FileOutLocation + "\\" + filehandler.FileInNameNoExtension + ".mobi");
                    }
                    //mobiApp.applicationArgString = filehandler.FileInEscaped;
                    //ExecuteExternalProcessor mobiProcessor = new ExecuteExternalProcessor(mobiApp);
                    break;
                case ".mobi":
                    string newTitle = Interaction.InputBox("Enter a New Title for This Book", "MetaData Manipulation", filehandler.FileInName, 100, 100);
                    mobi2mobiApp.applicationArgString = @"--title " + "\"" + newTitle + "\" " + filehandler.FileInEscaped + @" --outfile " + filehandler.FileInEscaped;
                    ExecuteExternalProcessor mobi2mobiProcessor = new ExecuteExternalProcessor(mobi2mobiApp);
                    break;
                case ".pdb":
                    pdbApp.applicationArgString = filehandler.FileInEscaped + " \"" + tempFolder + filehandler.FileInName + ".html\" \"1\"";
                    //MessageBox.Show(pdbApp.applicationArgString);
                    ExecuteExternalProcessor pdbProcessor = new ExecuteExternalProcessor(pdbApp);
                    if (FileConvert.MobiIt(tempFolder + filehandler.FileInName + ".html"))
                    {
                        GlobalTools.MoveFile(filehandler.FileInLocation + "\\" + filehandler.FileInName + ".mobi", filehandler.FileOutLocation + "\\" + filehandler.FileInName + ".mobi");
                    }
                    break;
                case ".lit":
                    string outPath = filehandler.TempLocation + filehandler.FileInNameNoExtension.Replace("'", "");
                    if(!Directory.Exists(outPath))
                    {
                        Directory.CreateDirectory(outPath);
                    }
                    clitApp.applicationArgString = "\"" + filehandler.FileInFullPath + "\" \"" + outPath + "\"";
                    ExecuteExternalProcessor litProcessor = new ExecuteExternalProcessor(clitApp);
                    if (GlobalTools.FileExist(outPath + "\\" + filehandler.FileInNameNoExtension + ".opf"))
                    {
                        if (FileConvert.MobiIt(outPath + "\\" + filehandler.FileInNameNoExtension + ".opf"))
                        {
                            GlobalTools.MoveFile(outPath + "\\" + filehandler.FileInNameNoExtension + ".mobi", filehandler.FileOutLocation + "\\" + filehandler.FileInNameNoExtension + ".mobi");
                        }
                    }
                    else
                    {
                        MessageBox.Show(litProcessor.GetapplicationErrorOutput(), "Lit Convert Failed");
                    }
                    break;
                case ".chm":
                    if (filehandler.FileInEscaped == "1")
                    {
                        chmApp.applicationArgString = filehandler.FileInEscaped + " \"" + tempFolder + "\"";
                        ExecuteExternalProcessor chmProcessor = new ExecuteExternalProcessor(chmApp);
                    }
                    break;
                case ".pdf":
                    pdfApp.applicationArgString = filehandler.FileInEscaped + " \"" + filehandler.TempLocation + "\\" + filehandler.FileInNameNoExtension +"\"";
                    ExecuteExternalProcessor pdfProcessor = new ExecuteExternalProcessor(pdfApp);
                    if (FileConvert.MobiIt(tempFolder + "\\" + filehandler.FileInNameNoExtension + "s.html"))
                    {
                        GlobalTools.MoveFile(tempFolder + "\\" + filehandler.FileInNameNoExtension + "s.mobi", filehandler.FileOutLocation + "\\" + filehandler.FileInNameNoExtension + ".mobi");
                    }
                    break;
                default:
                    MessageBox.Show("File type not supported", "Not Supported");
                    break;
            }
        }
    }
}
