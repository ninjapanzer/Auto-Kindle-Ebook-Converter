using System;
using System.Net;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using Global_Tools;

namespace AutoKindle_Updater
{
    class Ftp
    {
        public struct ftpCredentials
        {
            public string filePath;
            public string fileName;
            public string ftpServerIP;
            public string ftpUserID;
            public string ftpPassword;
        }

        private Queue<string> fileNameList = new Queue<string>();

        public ftpCredentials ftpCred = new ftpCredentials();
	    public Ftp()
	    {
            ftpCred.fileName = "";
            ftpCred.filePath = "";
            ftpCred.ftpServerIP = "";
            ftpCred.ftpUserID = "";
            ftpCred.ftpPassword = "";
	    }
        public Ftp(string filePath, string fileName, string ftpServerIP, string ftpUserID, string ftpPassword)
        {
            //ftpCredentials ftpCred = new ftpCredentials();
            ftpCred.filePath = filePath;
            ftpCred.fileName = fileName;
            ftpCred.ftpServerIP = ftpServerIP;
            ftpCred.ftpUserID = ftpUserID;
            ftpCred.ftpPassword = ftpPassword;
            getDirListFtp();
            getFileListFtp();
        }


        public Queue<string> getFileListQueue()
        {
            return this.fileNameList;
        }
        public static string dnsResolve(string ftpServerName)
        {
            string ftpServerIP;
            IPHostEntry ServerIP = Dns.GetHostEntry(ftpServerName);
            ftpServerIP = ServerIP.AddressList[0].ToString();
            return ftpServerIP;
        }
        public void getFileFtp()
        {
            FtpWebRequest reqFTP;
            try
            {
                //filePath: The full path where the file is to be created.
                //fileName: Name of the file to be createdNeed not name on
                //          the FTP server. name name()
                FileStream outputStream = new FileStream(this.ftpCred.filePath + "\\" + this.ftpCred.fileName, FileMode.Create);
                reqFTP = (FtpWebRequest)FtpWebRequest.Create(new Uri("ftp://" + this.ftpCred.ftpServerIP + "/" + this.ftpCred.fileName));
                reqFTP.Method = WebRequestMethods.Ftp.DownloadFile;
                reqFTP.UseBinary = true;
                reqFTP.Credentials = new NetworkCredential(this.ftpCred.ftpUserID, this.ftpCred.ftpPassword);
                FtpWebResponse response = (FtpWebResponse)reqFTP.GetResponse();
                Stream ftpStream = response.GetResponseStream();
                long cl = response.ContentLength;
                int bufferSize = 2048;
                int readCount;
                byte[] buffer = new byte[bufferSize];

                readCount = ftpStream.Read(buffer, 0, bufferSize);
                while (readCount > 0)
                {
                    outputStream.Write(buffer, 0, readCount);
                    readCount = ftpStream.Read(buffer, 0, bufferSize);
                }

                ftpStream.Close();
                outputStream.Close();
                response.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        public void getFileListFtp()
        {
            this.ftpCred.fileName = "currentlist.pnz";
            this.ftpCred.filePath = GlobalTools.GetexeLocation();
            this.getFileFtp();
            if (GlobalTools.FileExist(this.ftpCred.filePath + "//" + this.ftpCred.fileName))
            {
                string listInput;
                TextReader ReadText = new StreamReader(this.ftpCred.filePath + "//" + this.ftpCred.fileName);
                while ((listInput = ReadText.ReadLine()) != null)
                {
                    if (listInput != "FileList")
                    {
                        this.fileNameList.Enqueue(listInput);
                    }
                }
            }
        }
        public void getDirListFtp()
        {
            this.ftpCred.fileName = "directories.pnz";
            this.ftpCred.filePath = GlobalTools.GetexeLocation();
            this.getFileFtp();
            if (GlobalTools.FileExist(this.ftpCred.filePath + "//" + this.ftpCred.fileName))
            {
                string dirInput;
                TextReader ReadText = new StreamReader(this.ftpCred.filePath + "//" + this.ftpCred.fileName);
                while ((dirInput = ReadText.ReadLine()) != null)
                {
                    if (dirInput != "FileList")
                    {
                        Directory.CreateDirectory(this.ftpCred.filePath + "//" + dirInput);
                    }
                }
            }
        }
    }
}