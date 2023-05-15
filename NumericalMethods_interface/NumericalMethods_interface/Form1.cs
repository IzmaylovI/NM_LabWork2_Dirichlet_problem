using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;

namespace NumericalMethods_interface
{
    public partial class Form1 : Form
    {
        Random rand = new Random();
        DataTable values;
        string path_prog = "NumericalMethods_Lab2.exe";
        string path_input = "input.txt";
        string path_output = "result.txt";
        public Form1()
        {
            InitializeComponent();
        }
        private bool parseParams(global_data data) {
            data.numberVariant = 1;
            data.numberMethod = 1;
            data.a = 1;
            data.b = 2;
            data.c = 2;
            data.d = 3;
            data.area = "1";
            data.area_x = data.area_y = 1;
            data.param = 1.0;
            if (!int.TryParse(textBox_count.Text, out data.nmax)) MessageBox.Show("Incorrect N_max");
            else if (!double.TryParse(textBox_eps.Text, out data.eps)) MessageBox.Show("Incorrect eps");
            else if (!int.TryParse(textBox_n.Text, out data.n)) MessageBox.Show("Incorrect n");
            else if (!int.TryParse(textBox_m.Text, out data.m)) MessageBox.Show("Incorrect m");
            else if (comboBox2.SelectedIndex < 0) MessageBox.Show("Incorrect task");
            else if (!double.TryParse(textBox_param.Text, out data.param)) MessageBox.Show("Incorrect param");
            else
            {
                data.test = (comboBox2.SelectedIndex == 0);
                return true;
            }
            return false;
        }
        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                global_data data = new global_data();
                if (parseParams(data)) {
                    //MessageBox.Show(data.b.ToString());
                    string json = JsonConvert.SerializeObject(data);
                    FileStream fs = File.Open(path_input, FileMode.Create, FileAccess.Write);
                    fs.SetLength(0);
                    StreamWriter writer = new StreamWriter(fs);
                    writer.Write(json);
                    writer.Close();

                    if (backgroundWorker1.IsBusy) throw new Exception("backgroundWorker is busy");
                    else backgroundWorker1.RunWorkerAsync();
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
        }
        private void chart1_MouseClick(object sender, MouseEventArgs e)
        {
            //chart1.Refresh();
        }

        private int Clamp(int x, int min, int max) {
            if (x < min) x = min;
            if (x > max) x = max;
            return x;
        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            backgroundWorker1.ReportProgress(0);
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = path_prog;
            info.Arguments = path_input + " " + path_output;
            info.RedirectStandardError = true;
            info.RedirectStandardOutput = true;
            info.CreateNoWindow = true;
            info.UseShellExecute = false;
            StreamReader sr;
            Task<string> task = null;


            Process p = Process.Start(info);
            sr = p.StandardOutput;
            while ((!p.HasExited) || (!p.StandardOutput.EndOfStream))
            {
                if (backgroundWorker1.CancellationPending) { p.Kill(); e.Cancel = true; return; };
                if (task == null) task = p.StandardOutput.ReadLineAsync();
                if (task != null && task.IsCompleted) {
                    double stage = double.Parse(task.Result.Replace('.', ','));
                    task = null;
                    backgroundWorker1.ReportProgress(Clamp((int)(stage * 100), 0, 100));
                }
            }

            p.WaitForExit();
            if (p.ExitCode != 0) throw new Exception(p.StandardError.ReadLine());

            backgroundWorker1.ReportProgress(100);
        }

        private void print_ref_for_test_task(answer ans) {
            textBox_ref.Text = "";
            textBox_ref.Text += String.Format("Справка для тестовой задачи\r\n");
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("«Для решения тестовой задачи использованы сетка\r\n");
            textBox_ref.Text += String.Format("с числом разбиений по x n = {0}\tи числом разбиений по y m = {1},\r\n", ans.n, ans.m);
            textBox_ref.Text += String.Format("Использован {0} с параметром ω = {1}\r\n", "метод верхней релаксации", ans.param);
            textBox_ref.Text += String.Format("Применены критерии остановки:\r\n");
            textBox_ref.Text += String.Format(" по точности εмет = {0:e2}\r\n", ans.eps);
            textBox_ref.Text += String.Format(" по числу итераций Nmax = {0}\r\n", ans.nmax);
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("На решение схемы (СЛАУ) затрачено итераций N = {0} \r\n", ans.N);
            textBox_ref.Text += String.Format("и достигнута точность итерационного метода ε(N) = {0:e3}\r\n", ans.acc);
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("Схема (СЛАУ) решена с невязкой ||R(N)|| = {0:e3} по {1}\r\n", ans.R, "норме max");
            textBox_ref.Text += String.Format("для невязки СЛАУ использована {0}\r\n", "норма max");
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("Тестовая задача должна быть решена с погрешностью не более ε = 0.5*10^–6;\r\n");
            textBox_ref.Text += String.Format("задача решена с погрешностью ε1 = {0:e3}\r\n", ans.err);
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("Максимальное отклонение точного и численного решений наблюдается в узле x = {0:f3}; y = {1:f3}\r\n", ans.x, ans.y);
            textBox_ref.Text += String.Format("\r\n");
            textBox_ref.Text += String.Format("В качестве начального приближения использовано {0}\r\n", "интерполяция по x");
        }

        private void backgroundWorker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!e.Cancelled) {
                //MessageBox.Show(backgroundWorker1.CancellationPending.ToString());
                FileStream fs = File.Open(path_output, FileMode.Open, FileAccess.Read);
                StreamReader reader = new StreamReader(fs);
                answer ans = JsonConvert.DeserializeObject<answer>(reader.ReadToEnd());
                reader.Close();

                //values.Rows.Clear();
                //values.Columns.Clear();

                values = new DataTable();
                dataGridView1.DataSource = values;
                //columns
                values.Columns.Add(String.Format("x[i]"), typeof(string));
                for (int i = 0; i <= ans.n; ++i) values.Columns.Add(String.Format("x[{0}]", i), typeof(string));
                //rows
                values.Rows.Add();
                dataGridView1.Rows[0].HeaderCell.Value = String.Format("y[j]");
                for (int j = 0; j <= ans.m; ++j) {
                    values.Rows.Add();
                    dataGridView1.Rows[j + 1].HeaderCell.Value = String.Format("y[{0}]", j);
                }


                dataGridView1[0, 0].Value = String.Format("Y\\X");


                for (int i = 0; i <= ans.n; ++i) 
                {
                    dataGridView1[i+1, 0].Value = String.Format("{0:f3}", ans.coord_x[i]);
                }

                for (int j = 0; j <= ans.m; ++j)
                {
                    dataGridView1[0, j + 1].Value = String.Format("{0:f3}", ans.coord_y[j]);
                }


                for (int j = 0; j <= ans.m; ++j) {
                    for (int i = 0; i <= ans.n; ++i) {
                        dataGridView1[i + 1, j + 1].Value = String.Format("{0:f3}", ans.v[j][i]);
                    }
                }
                print_ref_for_test_task(ans);
            }
        }

        private void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            progressBar1.Value = e.ProgressPercentage;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            backgroundWorker1.CancelAsync();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            backgroundWorker1.CancelAsync();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }
    }

    class global_data
    {
        public int numberVariant;
        public int numberMethod;
        public bool test;
        // stop condition
        public int nmax;
        public double eps;
        // area
        public int n, m;
        public double a, b, c, d;
        public string area;
        public int area_x, area_y;
        // method
        public double param;
    }

    class answer
    {
        public int n, m;
        public int N;
        public int nmax;
        public double eps;
        public double acc;
        public double err;
        public double R;
        public double x, y;
        public double param;
        public List<List<double>> v;
        public List<double> coord_x;
        public List<double> coord_y;
    }
}
