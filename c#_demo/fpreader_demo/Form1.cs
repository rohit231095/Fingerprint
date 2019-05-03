using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Data.OleDb;
using System.Runtime.InteropServices;
using System.IO;

namespace fpreader_demo
{
    public partial class Form1 : Form
    {
        string strFPImagepath;
        OleDbConnection myAccessConn;

        public Form1()
        {
            InitializeComponent();
            Message.Text = "TIMMY DEMO";

            strFPImagepath = System.Environment.CurrentDirectory;
            strFPImagepath += "\\test.bmp";

            //             pictureBox.Load(strFPImagepath);
            //             pictureBox.Show();

            TB_ID.Text = "2";
            TB_FP_NUM.Text = "0";
            TB_Privilege.Text = "0";


        }

        private void detectDevice()
        {
            bool bRet=false;
            do
            {
                bRet = fpreaderdll.DetectFinger();
                if (bRet)
                {
                    break;
                }
            } while (true);
        }

        private void showFPPicture()
        {
            FileStream fs = new FileStream(strFPImagepath, FileMode.Open, FileAccess.Read);
            pictureBox.Image = Image.FromStream(fs);
            pictureBox.Show();
            pictureBox.Refresh();
            fs.Close();
        }

        private void CloseDevice()
        {
            bool bRet;

            bRet = fpreaderdll.OnoffLED(0);
            bRet = fpreaderdll.DisConnectUsb();
        }

        private void OpenDB()
        {
            string strConnection = @"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=.\EnrollData.mdb";
            myAccessConn = new OleDbConnection(strConnection);
            myAccessConn.Open();

            if (myAccessConn.State != ConnectionState.Open)
            {
                MessageBox.Show("Access数据库的连接失败!", "Access数据库的连接");
                return;
            }
        }

        private void CloseDB()
        {
            if (myAccessConn.State == ConnectionState.Open)
            {
                myAccessConn.Close();
            }
        }

        // delete all data in DB
        private void button1_Click(object sender, EventArgs e)
        {
            bool bRet;
            int verIndex = 0;
            StringBuilder strVersion = new StringBuilder(30);

            int length = 0;
            StringBuilder  fpdatastr =new StringBuilder(2000); 


            bRet = fpreaderdll.ConnectUsb();
            if (!bRet)
            {

                Message.Text = "No device";
                Message.Update();

                return;
            }

            bRet = fpreaderdll.Getalgorithm(ref verIndex,  strVersion);
            if (!bRet)
            {
                Message.Text = "No device";
                Message.Update();

                return;
            }

            fpreaderdll.OnoffLED(1);  //enable
            Message.Text = "press finger 1#";
            Message.Update();

            detectDevice();


            bRet = fpreaderdll.GetImage(strFPImagepath);
            if (bRet)
            {
                showFPPicture();

            }
            else
            {
                Message.Text = "Get image fail";
                Message.Update();

                CloseDevice();
            }

            bRet = fpreaderdll.GetTemplet(0);
            if (!bRet)
            {
                Message.Text = "Get Templet Fail";
                Message.Update();

                CloseDevice();
                return;
            }

            //int dwEMachineNumber;
            //int dwEnrollNumber;
            int dwFingerNumber;
            //int dwPrivilege;
            

            OpenDB();
            string strAccessSelect = "SELECT * FROM tblEnroll";
            OleDbCommand myAccessCommand = new OleDbCommand(strAccessSelect, myAccessConn);
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter(myAccessCommand);
            DataSet myDataSet = new DataSet();

            myDataAdapter.Fill(myDataSet, "Categories");

            DataRowCollection dra = myDataSet.Tables["Categories"].Rows;
            if (dra.Count == 0)
            {
                Message.Text = "FP DataBase is empty.";
                Message.Update();
                goto SKIP;
            }
            string str;
            object objFPData;
            // check duplicated FPData
            foreach (DataRow dr in dra)
            {
                dwFingerNumber = (int)dr["FingerNumber"];
                str = dr["FPData_Str"].GetType().ToString();
                if (str == "System.DBNull")
                {
                    continue;
                }
                else
                {
                    objFPData = dr["FPData_Str"];
                    if (objFPData.ToString() == "")  // this can be happen
                    {
                        continue;
                    }
                }

                if (dwFingerNumber < 10 )
                {

                    bRet = fpreaderdll.VerifyFinger(objFPData.ToString());
                    if (bRet)
                    {
                        Message.Text = "FP DOUBLE";  // duplicate
                        Message.Update();
                        CloseDevice();
                        CloseDB();
                        return;
                    }
                } // if


            } // foreach

SKIP:
            CloseDB();

            Message.Text = "press figer 2 #";
            Message.Update();
            fpreaderdll.OnoffLED(1);  //enable
            detectDevice();

            bRet = fpreaderdll.GetImage(strFPImagepath);
            if (bRet)
            {

                showFPPicture();
            }
            else
            {
                Message.Text = "Get image fail";
                Message.Update();

                CloseDevice();
                return;
            }

            fpreaderdll.GetTemplet(1);
            if (!bRet)
            {
                Message.Text = "Get Templet Fail";
                Message.Update();

                CloseDevice();
                return;
            }

            if (verIndex == 1)
            {
                Message.Text = "press figer 3 #";
                Message.Update();

                detectDevice();

                bRet = fpreaderdll.GetImage(strFPImagepath);
                if (bRet)
                {

                    showFPPicture();
                }
                else
                {
                    Message.Text = "Get image fail";
                    Message.Update();

                    CloseDevice();
                    return;
                }

                bRet = fpreaderdll.GetTemplet(2);
                if (!bRet)
                {
                    Message.Text = "Get Templet Fail";
                    Message.Update();

                    CloseDevice();
                    return;
                }

            }  // if (verIndex == 1)

            bRet = fpreaderdll.GetMergeChar(fpdatastr);
            if (bRet == false)
            {
                Message.Text = "MergeChar Error";
                Message.Update();

                CloseDevice();
                return;
            }

            OleDbParameter[] parameters = new OleDbParameter[6];
            int index = 0;
            parameters[index] = new OleDbParameter("@EMachineNumber", OleDbType.Integer);
            parameters[index].Value = 1;
            index++;

            parameters[index] = new OleDbParameter("@EnrollNumber", OleDbType.Integer);
            parameters[index].Value = Int32.Parse(TB_ID.Text.ToString());
            index++;

            parameters[index] = new OleDbParameter("@FingerNumber", OleDbType.Integer);
            parameters[index].Value = Int32.Parse(TB_FP_NUM.Text.ToString());
            index++;

            parameters[index] = new OleDbParameter("@Privilige", OleDbType.Integer);
            parameters[index].Value = Int32.Parse(TB_Privilege.Text.ToString());
            index++;

            parameters[index] = new OleDbParameter("@enPassword", OleDbType.Integer);
            parameters[index].Value = 0;
            index++;

            
            parameters[index] = new OleDbParameter("@FPData_Str", OleDbType.BSTR);
            parameters[index].Value = fpdatastr.ToString();   //accept byte[]
            index++;

            //             parameters[index] = new OleDbParameter("@EnrollName", OleDbType.BSTR);
            //             parameters[index].Value = "";   //accept byte[]
            //             index++;
            // 
            //             parameters[index] = new OleDbParameter("@FPData_Str", OleDbType.BSTR);
            //             parameters[index].Value = 0;   //accept byte[]

            OpenDB();
            string sqlEx = "insert into tblEnroll(EMachineNumber,EnrollNumber,FingerNumber,Privilige,enPassword,FPData_Str)" +
                       "values(@EMachineNumber,@EnrollNumber,@FingerNumber,@Privilige,?,@FPData_Str)";

            OleDbCommand cmd = new OleDbCommand(sqlEx, myAccessConn);
            try
            {
                //conn.Open();
                if (parameters != null)
                {
                    cmd.Parameters.AddRange(parameters);
                }
                cmd.ExecuteNonQuery();

            }

            catch (OleDbException eDB)
            {
                string errorMessages = "";

                for (int i = 0; i < eDB.Errors.Count; i++)
                {
                    errorMessages += "Index #" + i + "\n" +
                                     "Message: " + eDB.Errors[i].Message + "\n" +
                                     "NativeError: " + eDB.Errors[i].NativeError + "\n" +
                                     "Source: " + eDB.Errors[i].Source + "\n" +
                                     "SQLState: " + eDB.Errors[i].SQLState + "\n";
                }

                System.Diagnostics.EventLog log = new System.Diagnostics.EventLog();
                log.Source = "My Application";
                log.WriteEntry(errorMessages);
                Console.WriteLine("An exception occurred. Please contact your system administrator.");

                CloseDB();

                Message.Text = "fail to manipulate DB..... ";
                Message.Update();
                return;
            }
            catch (Exception ec)
            {
                throw ec;
            }

            CloseDB();

            Message.Text = "added new FPData to DB..... ";
            Message.Update();
        }

        private void button3_Click_1(object sender, EventArgs e)
        {
            DialogResult dr;
            dr = MessageBox.Show("Continue?", "Delete All data in database? ", MessageBoxButtons.YesNo, MessageBoxIcon.Asterisk);
            if (dr == DialogResult.No)
            {
                return;
            }
            OpenDB();

            string sql = "delete * from tblEnroll";
            OleDbCommand cmd = new OleDbCommand(sql, myAccessConn);

            try
            {
                cmd.ExecuteNonQuery();
            }
            catch (Exception ec)
            {
                throw ec;
            }
            CloseDB();


            Message.Text = "deleted all data in DB..... ";
            Message.Update();
        }
    }
}
