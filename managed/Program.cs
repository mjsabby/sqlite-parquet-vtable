namespace ParquetSQLite
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using static NativeMethods;

    public sealed class Results
    {
        public List<string> ColumnNames { get; set; }

        public List<List<string>> Rows { get; set; }
    }

    public unsafe static class NativeMethods
    {
        private const string DllPath = @"ParquetSQLite";

        [DllImport(DllPath)]
        public static extern void ParquetSQLiteProcessWideInit(void* tmp);

        [DllImport(DllPath, CharSet = CharSet.Ansi)]
        public static extern int ParquetSQLiteOpenDatabase(string path, out IntPtr sqlite);

        [DllImport(DllPath)]
        public static extern int ParquetSQLiteCloseDatabase(IntPtr sqlite);

        [DllImport(DllPath, CharSet = CharSet.Ansi)]
        public static extern int ParquetSQLiteExec(IntPtr sqlite, string sql, delegate* unmanaged[Stdcall]<IntPtr, int, IntPtr, IntPtr, int> callback, IntPtr callbackData, out IntPtr errorMessage);

        [DllImport(DllPath)]
        public static extern void ParquetSQLiteFreeMemory(IntPtr ptr);
    }

    public unsafe static class Program
    {
        [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
        public static int ResultCallback(IntPtr context, int numColumns, IntPtr columnValues, IntPtr columnNames)
        {
            var results = (Results)GCHandle.FromIntPtr(context).Target;

            if (results.ColumnNames == null)
            {
                results.ColumnNames = new List<string>();
                var columnNamesSpan = new Span<IntPtr>((void*)columnNames, numColumns);

                for (int i = 0; i < numColumns; ++i)
                {
                    results.ColumnNames.Add(Marshal.PtrToStringAnsi(columnNamesSpan[i]));
                }
            }

            var columnValuesSpan = new Span<IntPtr>((void*)columnValues, numColumns);
            var list = new List<string>();
            for (int i = 0; i < numColumns; ++i)
            {
                list.Add(Marshal.PtrToStringAnsi(columnValuesSpan[i]));
            }

            results.Rows.Add(list);

            return 0; // returning a non-zero value terminates result set enumeration
        }

        public static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine(@"Usage: ParquetSQLiteTool ""'C:\test1.parquet', 'C:\test2.parquet'"" ""SELECT COUNT(*) FROM ParquetFiles""");
                Console.WriteLine(@"NOTE: In this convenience tool there is always one table representing the parquet files and it is called 'ParquetFiles'");
                return;
            }

            const int MaxLength = 1024;
            Span<byte> tempPathOnStack = stackalloc byte[MaxLength];

            var sqlite = IntPtr.Zero;
            var tempPath = Path.GetTempPath();

            var tempPathSize = Encoding.UTF8.GetBytes(tempPath, tempPathOnStack);
            if (tempPathSize == MaxLength)
            {
                throw new Exception($"Temporary path is {MaxLength} characters which is too long.");
            }

            var tempFile = Path.Combine(tempPath, Path.GetRandomFileName());

            fixed (byte* tempPathPtr = &MemoryMarshal.GetReference(tempPathOnStack))
            {
                try
                {
                    ParquetSQLiteProcessWideInit(tempPathPtr);

                    if (ParquetSQLiteOpenDatabase(tempFile, out sqlite) != 0)
                    {
                        Console.WriteLine($"Unable to open a temp SQLite database: {tempFile}");
                        return;
                    }

                    if (ParquetSQLiteExec(sqlite, @"CREATE VIRTUAL TABLE ParquetFiles USING parquet(" + args[0] + ")", &ResultCallback, IntPtr.Zero, out IntPtr errorMessage) != 0)
                    {
                        if (errorMessage != IntPtr.Zero)
                        {
                            Console.WriteLine($"Error from SQLite: {Marshal.PtrToStringAnsi(errorMessage)}");
                            ParquetSQLiteFreeMemory(errorMessage);
                            return;
                        }
                    }

                    var results = new Results { Rows = new List<List<string>>() };
                    if (ParquetSQLiteExec(sqlite, args[1], &ResultCallback, GCHandle.ToIntPtr(GCHandle.Alloc(results, GCHandleType.Normal)), out errorMessage) != 0)
                    {
                        if (errorMessage != IntPtr.Zero)
                        {
                            Console.WriteLine($"Error from SQLite: {Marshal.PtrToStringAnsi(errorMessage)}");
                            ParquetSQLiteFreeMemory(errorMessage);
                            return;
                        }
                    }

                    var columnNames = results.ColumnNames;
                    if (columnNames != null)
                    {
                        for (int i = 0; i < columnNames.Count; ++i)
                        {
                            Console.Write($"| {columnNames[i]} |\t");
                        }

                        Console.WriteLine();

                        var rows = results.Rows;
                        for (int i = 0; i < rows.Count; ++i)
                        {
                            var row = rows[i];
                            for (int j = 0; j < columnNames.Count; ++j)
                            {
                                Console.Write($"| {row[j]} |\t");
                            }

                            Console.WriteLine();
                        }
                    }
                }
                finally
                {
                    if (sqlite != IntPtr.Zero)
                    {
                        ParquetSQLiteCloseDatabase(sqlite);
                    }

                    if (File.Exists(tempFile))
                    {
                        File.Delete(tempFile);
                    }
                }
            }
        }
    }
}