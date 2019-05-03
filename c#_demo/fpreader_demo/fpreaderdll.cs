using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace fpreader_demo
{
    class fpreaderdll
    {
        [DllImport("fpreader.dll", EntryPoint = "GetLastError", CallingConvention = CallingConvention.Winapi)]
        public static extern bool GetLastError(Int32 dwErrorCode);

        [DllImport("fpreader.dll", EntryPoint = "ConnectUsb", CallingConvention = CallingConvention.Winapi)]
        public static extern bool ConnectUsb();

        [DllImport("fpreader.dll", EntryPoint = "DisConnectUsb", CallingConvention = CallingConvention.Winapi)]
        public static extern bool DisConnectUsb();

        [DllImport("fpreader.dll", EntryPoint = "Getalgorithm", CallingConvention = CallingConvention.Winapi)]
        public static extern bool Getalgorithm(ref Int32 versionindex,  StringBuilder versionchar);

        [DllImport("fpreader.dll", EntryPoint = "OnoffLED", CallingConvention = CallingConvention.Winapi)]
        public static extern bool OnoffLED(Int32 onoff);

        [DllImport("fpreader.dll", EntryPoint = "DetectFinger", CallingConvention = CallingConvention.Winapi)]
        public static extern bool DetectFinger();

        [DllImport("fpreader.dll", EntryPoint = "DetectCard", CallingConvention = CallingConvention.Winapi)]
        public static extern bool DetectCard(ref IntPtr CardNum);

        [DllImport("fpreader.dll", EntryPoint = "GetTemplet", CallingConvention = CallingConvention.Winapi)]
        public static extern bool GetTemplet(Int32 step);

        [DllImport("fpreader.dll", EntryPoint = "GetImage", CallingConvention = CallingConvention.Winapi)]
        public static extern bool GetImage(string LPSZFileName);

        [DllImport("fpreader.dll", EntryPoint = "GetMergeChar", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Winapi)]
        public static extern bool GetMergeChar(StringBuilder dwEnrollData);

        [DllImport("fpreader.dll", EntryPoint = "VerifyFinger", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Winapi)]
        public static extern bool VerifyFinger(string dwEnrollData);

        [DllImport("fpreader.dll", EntryPoint = "MatchTwoFinger", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Winapi)]
        public static extern bool MatchTwoFinger(string dwEnrollData1, string dwEnrollData2);
    }
}
