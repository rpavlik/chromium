namespace FlockOfBirds
{
    partial class GUI
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
          this.menuStrip = new System.Windows.Forms.MenuStrip();
          this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
          this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
          this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
          this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
          this.statusStrip = new System.Windows.Forms.StatusStrip();
          this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
          this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
          this.textBox1 = new System.Windows.Forms.TextBox();
          this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
          this.gbData = new System.Windows.Forms.GroupBox();
          this.pPreview = new System.Windows.Forms.Panel();
          this.btnMute = new System.Windows.Forms.Button();
          this.btnPause = new System.Windows.Forms.Button();
          this.btnSend = new System.Windows.Forms.Button();
          this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
          this.eRot11 = new System.Windows.Forms.TextBox();
          this.eRot12 = new System.Windows.Forms.TextBox();
          this.eRot13 = new System.Windows.Forms.TextBox();
          this.eRot21 = new System.Windows.Forms.TextBox();
          this.eRot22 = new System.Windows.Forms.TextBox();
          this.eRot23 = new System.Windows.Forms.TextBox();
          this.eRot31 = new System.Windows.Forms.TextBox();
          this.eRot32 = new System.Windows.Forms.TextBox();
          this.eRot33 = new System.Windows.Forms.TextBox();
          this.eZang = new System.Windows.Forms.TextBox();
          this.eYang = new System.Windows.Forms.TextBox();
          this.eXang = new System.Windows.Forms.TextBox();
          this.eZPos = new System.Windows.Forms.TextBox();
          this.eYPos = new System.Windows.Forms.TextBox();
          this.eXPos = new System.Windows.Forms.TextBox();
          this.eTimeStamp = new System.Windows.Forms.TextBox();
          this.label8 = new System.Windows.Forms.Label();
          this.label7 = new System.Windows.Forms.Label();
          this.label6 = new System.Windows.Forms.Label();
          this.label5 = new System.Windows.Forms.Label();
          this.label4 = new System.Windows.Forms.Label();
          this.label3 = new System.Windows.Forms.Label();
          this.label2 = new System.Windows.Forms.Label();
          this.label1 = new System.Windows.Forms.Label();
          this.gbSettings = new System.Windows.Forms.GroupBox();
          this.udPort = new System.Windows.Forms.NumericUpDown();
          this.udMRate = new System.Windows.Forms.NumericUpDown();
          this.cbHemisphere = new System.Windows.Forms.ComboBox();
          this.cbBaudRate = new System.Windows.Forms.ComboBox();
          this.cbComPort = new System.Windows.Forms.ComboBox();
          this.btnApply = new System.Windows.Forms.Button();
          this.eIPAddress = new System.Windows.Forms.TextBox();
          this.label15 = new System.Windows.Forms.Label();
          this.label14 = new System.Windows.Forms.Label();
          this.label13 = new System.Windows.Forms.Label();
          this.label12 = new System.Windows.Forms.Label();
          this.label11 = new System.Windows.Forms.Label();
          this.label10 = new System.Windows.Forms.Label();
          this.menuStrip.SuspendLayout();
          this.statusStrip.SuspendLayout();
          this.tableLayoutPanel2.SuspendLayout();
          this.gbData.SuspendLayout();
          this.tableLayoutPanel3.SuspendLayout();
          this.gbSettings.SuspendLayout();
          ((System.ComponentModel.ISupportInitialize)(this.udPort)).BeginInit();
          ((System.ComponentModel.ISupportInitialize)(this.udMRate)).BeginInit();
          this.SuspendLayout();
          // 
          // menuStrip
          // 
          this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
          this.menuStrip.Location = new System.Drawing.Point(0, 0);
          this.menuStrip.Name = "menuStrip";
          this.menuStrip.Size = new System.Drawing.Size(602, 24);
          this.menuStrip.TabIndex = 0;
          this.menuStrip.Text = "menuStrip1";
          // 
          // fileToolStripMenuItem
          // 
          this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
          this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
          this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
          this.fileToolStripMenuItem.Text = "&File";
          // 
          // exitToolStripMenuItem
          // 
          this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
          this.exitToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
          this.exitToolStripMenuItem.Text = "E&xit";
          this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
          // 
          // helpToolStripMenuItem
          // 
          this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
          this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
          this.helpToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
          this.helpToolStripMenuItem.Text = "&Help";
          // 
          // aboutToolStripMenuItem
          // 
          this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
          this.aboutToolStripMenuItem.Size = new System.Drawing.Size(126, 22);
          this.aboutToolStripMenuItem.Text = "&About...";
          this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
          // 
          // statusStrip
          // 
          this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel});
          this.statusStrip.Location = new System.Drawing.Point(0, 471);
          this.statusStrip.Name = "statusStrip";
          this.statusStrip.Size = new System.Drawing.Size(602, 22);
          this.statusStrip.TabIndex = 4;
          this.statusStrip.Text = "statusStrip1";
          // 
          // toolStripStatusLabel
          // 
          this.toolStripStatusLabel.Name = "toolStripStatusLabel";
          this.toolStripStatusLabel.Size = new System.Drawing.Size(109, 17);
          this.toolStripStatusLabel.Text = "toolStripStatusLabel1";
          // 
          // tableLayoutPanel1
          // 
          this.tableLayoutPanel1.ColumnCount = 2;
          this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 311F));
          this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 66.66666F));
          this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
          this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
          this.tableLayoutPanel1.Name = "tableLayoutPanel1";
          this.tableLayoutPanel1.RowCount = 2;
          this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
          this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
          this.tableLayoutPanel1.Size = new System.Drawing.Size(200, 100);
          this.tableLayoutPanel1.TabIndex = 0;
          // 
          // textBox1
          // 
          this.textBox1.Dock = System.Windows.Forms.DockStyle.Fill;
          this.textBox1.Location = new System.Drawing.Point(3, 3);
          this.textBox1.Multiline = true;
          this.textBox1.Name = "textBox1";
          this.textBox1.Size = new System.Drawing.Size(194, 94);
          this.textBox1.TabIndex = 2;
          // 
          // tableLayoutPanel2
          // 
          this.tableLayoutPanel2.ColumnCount = 2;
          this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
          this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 305F));
          this.tableLayoutPanel2.Controls.Add(this.gbData, 0, 0);
          this.tableLayoutPanel2.Controls.Add(this.gbSettings, 1, 0);
          this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
          this.tableLayoutPanel2.Location = new System.Drawing.Point(0, 24);
          this.tableLayoutPanel2.Name = "tableLayoutPanel2";
          this.tableLayoutPanel2.RowCount = 1;
          this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 415F));
          this.tableLayoutPanel2.Size = new System.Drawing.Size(602, 447);
          this.tableLayoutPanel2.TabIndex = 7;
          // 
          // gbData
          // 
          this.gbData.Controls.Add(this.pPreview);
          this.gbData.Controls.Add(this.btnMute);
          this.gbData.Controls.Add(this.btnPause);
          this.gbData.Controls.Add(this.btnSend);
          this.gbData.Controls.Add(this.tableLayoutPanel3);
          this.gbData.Controls.Add(this.eZang);
          this.gbData.Controls.Add(this.eYang);
          this.gbData.Controls.Add(this.eXang);
          this.gbData.Controls.Add(this.eZPos);
          this.gbData.Controls.Add(this.eYPos);
          this.gbData.Controls.Add(this.eXPos);
          this.gbData.Controls.Add(this.eTimeStamp);
          this.gbData.Controls.Add(this.label8);
          this.gbData.Controls.Add(this.label7);
          this.gbData.Controls.Add(this.label6);
          this.gbData.Controls.Add(this.label5);
          this.gbData.Controls.Add(this.label4);
          this.gbData.Controls.Add(this.label3);
          this.gbData.Controls.Add(this.label2);
          this.gbData.Controls.Add(this.label1);
          this.gbData.Dock = System.Windows.Forms.DockStyle.Fill;
          this.gbData.Location = new System.Drawing.Point(3, 3);
          this.gbData.MinimumSize = new System.Drawing.Size(300, 0);
          this.gbData.Name = "gbData";
          this.gbData.Size = new System.Drawing.Size(300, 441);
          this.gbData.TabIndex = 3;
          this.gbData.TabStop = false;
          this.gbData.Text = "Data";
          // 
          // pPreview
          // 
          this.pPreview.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                      | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.pPreview.BackColor = System.Drawing.Color.Black;
          this.pPreview.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
          this.pPreview.Location = new System.Drawing.Point(92, 300);
          this.pPreview.Name = "pPreview";
          this.pPreview.Size = new System.Drawing.Size(202, 106);
          this.pPreview.TabIndex = 21;
          this.pPreview.Resize += new System.EventHandler(this.pPreview_Resize);
          this.pPreview.Paint += new System.Windows.Forms.PaintEventHandler(this.pPreview_Paint);
          // 
          // btnMute
          // 
          this.btnMute.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
          this.btnMute.Location = new System.Drawing.Point(57, 412);
          this.btnMute.Name = "btnMute";
          this.btnMute.Size = new System.Drawing.Size(75, 23);
          this.btnMute.TabIndex = 17;
          this.btnMute.Text = "Mute";
          this.btnMute.UseVisualStyleBackColor = true;
          this.btnMute.Click += new System.EventHandler(this.btnMute_Click);
          // 
          // btnPause
          // 
          this.btnPause.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
          this.btnPause.Location = new System.Drawing.Point(138, 412);
          this.btnPause.Name = "btnPause";
          this.btnPause.Size = new System.Drawing.Size(75, 23);
          this.btnPause.TabIndex = 18;
          this.btnPause.Text = "Pause";
          this.btnPause.UseVisualStyleBackColor = true;
          this.btnPause.Click += new System.EventHandler(this.btnPause_Click);
          // 
          // btnSend
          // 
          this.btnSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
          this.btnSend.Enabled = false;
          this.btnSend.Location = new System.Drawing.Point(219, 412);
          this.btnSend.Name = "btnSend";
          this.btnSend.Size = new System.Drawing.Size(75, 23);
          this.btnSend.TabIndex = 19;
          this.btnSend.Text = "Send";
          this.btnSend.UseVisualStyleBackColor = true;
          this.btnSend.Click += new System.EventHandler(this.btnSend_Click);
          // 
          // tableLayoutPanel3
          // 
          this.tableLayoutPanel3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.tableLayoutPanel3.ColumnCount = 3;
          this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.Controls.Add(this.eRot11, 0, 0);
          this.tableLayoutPanel3.Controls.Add(this.eRot12, 1, 0);
          this.tableLayoutPanel3.Controls.Add(this.eRot13, 2, 0);
          this.tableLayoutPanel3.Controls.Add(this.eRot21, 0, 1);
          this.tableLayoutPanel3.Controls.Add(this.eRot22, 1, 1);
          this.tableLayoutPanel3.Controls.Add(this.eRot23, 2, 1);
          this.tableLayoutPanel3.Controls.Add(this.eRot31, 0, 2);
          this.tableLayoutPanel3.Controls.Add(this.eRot32, 1, 2);
          this.tableLayoutPanel3.Controls.Add(this.eRot33, 2, 2);
          this.tableLayoutPanel3.Location = new System.Drawing.Point(92, 230);
          this.tableLayoutPanel3.Margin = new System.Windows.Forms.Padding(0);
          this.tableLayoutPanel3.Name = "tableLayoutPanel3";
          this.tableLayoutPanel3.RowCount = 3;
          this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
          this.tableLayoutPanel3.Size = new System.Drawing.Size(202, 67);
          this.tableLayoutPanel3.TabIndex = 16;
          // 
          // eRot11
          // 
          this.eRot11.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot11.Enabled = false;
          this.eRot11.Location = new System.Drawing.Point(0, 0);
          this.eRot11.Margin = new System.Windows.Forms.Padding(0);
          this.eRot11.Multiline = true;
          this.eRot11.Name = "eRot11";
          this.eRot11.Size = new System.Drawing.Size(67, 22);
          this.eRot11.TabIndex = 8;
          this.eRot11.Text = "1.0";
          // 
          // eRot12
          // 
          this.eRot12.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot12.Enabled = false;
          this.eRot12.Location = new System.Drawing.Point(67, 0);
          this.eRot12.Margin = new System.Windows.Forms.Padding(0);
          this.eRot12.Multiline = true;
          this.eRot12.Name = "eRot12";
          this.eRot12.Size = new System.Drawing.Size(67, 22);
          this.eRot12.TabIndex = 9;
          this.eRot12.Text = "0.0";
          // 
          // eRot13
          // 
          this.eRot13.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot13.Enabled = false;
          this.eRot13.Location = new System.Drawing.Point(134, 0);
          this.eRot13.Margin = new System.Windows.Forms.Padding(0);
          this.eRot13.Multiline = true;
          this.eRot13.Name = "eRot13";
          this.eRot13.Size = new System.Drawing.Size(68, 22);
          this.eRot13.TabIndex = 2;
          this.eRot13.Text = "0.0";
          // 
          // eRot21
          // 
          this.eRot21.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot21.Enabled = false;
          this.eRot21.Location = new System.Drawing.Point(0, 22);
          this.eRot21.Margin = new System.Windows.Forms.Padding(0);
          this.eRot21.Multiline = true;
          this.eRot21.Name = "eRot21";
          this.eRot21.Size = new System.Drawing.Size(67, 22);
          this.eRot21.TabIndex = 10;
          this.eRot21.Text = "0.0";
          // 
          // eRot22
          // 
          this.eRot22.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot22.Enabled = false;
          this.eRot22.Location = new System.Drawing.Point(67, 22);
          this.eRot22.Margin = new System.Windows.Forms.Padding(0);
          this.eRot22.Multiline = true;
          this.eRot22.Name = "eRot22";
          this.eRot22.Size = new System.Drawing.Size(67, 22);
          this.eRot22.TabIndex = 11;
          this.eRot22.Text = "1.0";
          // 
          // eRot23
          // 
          this.eRot23.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot23.Enabled = false;
          this.eRot23.Location = new System.Drawing.Point(134, 22);
          this.eRot23.Margin = new System.Windows.Forms.Padding(0);
          this.eRot23.Multiline = true;
          this.eRot23.Name = "eRot23";
          this.eRot23.Size = new System.Drawing.Size(68, 22);
          this.eRot23.TabIndex = 12;
          this.eRot23.Text = "0.0";
          // 
          // eRot31
          // 
          this.eRot31.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot31.Enabled = false;
          this.eRot31.Location = new System.Drawing.Point(0, 44);
          this.eRot31.Margin = new System.Windows.Forms.Padding(0);
          this.eRot31.Multiline = true;
          this.eRot31.Name = "eRot31";
          this.eRot31.Size = new System.Drawing.Size(67, 23);
          this.eRot31.TabIndex = 13;
          this.eRot31.Text = "0.0";
          // 
          // eRot32
          // 
          this.eRot32.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot32.Enabled = false;
          this.eRot32.Location = new System.Drawing.Point(67, 44);
          this.eRot32.Margin = new System.Windows.Forms.Padding(0);
          this.eRot32.Multiline = true;
          this.eRot32.Name = "eRot32";
          this.eRot32.Size = new System.Drawing.Size(67, 23);
          this.eRot32.TabIndex = 14;
          this.eRot32.Text = "0.0";
          // 
          // eRot33
          // 
          this.eRot33.Dock = System.Windows.Forms.DockStyle.Fill;
          this.eRot33.Enabled = false;
          this.eRot33.Location = new System.Drawing.Point(134, 44);
          this.eRot33.Margin = new System.Windows.Forms.Padding(0);
          this.eRot33.Multiline = true;
          this.eRot33.Name = "eRot33";
          this.eRot33.Size = new System.Drawing.Size(68, 23);
          this.eRot33.TabIndex = 15;
          this.eRot33.Text = "1.0";
          // 
          // eZang
          // 
          this.eZang.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eZang.Enabled = false;
          this.eZang.Location = new System.Drawing.Point(92, 194);
          this.eZang.Name = "eZang";
          this.eZang.Size = new System.Drawing.Size(202, 20);
          this.eZang.TabIndex = 7;
          this.eZang.Tag = "2";
          this.eZang.Text = "0.0";
          this.eZang.Validated += new System.EventHandler(this.eZang_Validated);
          this.eZang.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eYang
          // 
          this.eYang.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eYang.Enabled = false;
          this.eYang.Location = new System.Drawing.Point(92, 168);
          this.eYang.Name = "eYang";
          this.eYang.Size = new System.Drawing.Size(202, 20);
          this.eYang.TabIndex = 6;
          this.eYang.Tag = "1";
          this.eYang.Text = "0.0";
          this.eYang.Validated += new System.EventHandler(this.eYang_Validated);
          this.eYang.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eXang
          // 
          this.eXang.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eXang.Enabled = false;
          this.eXang.Location = new System.Drawing.Point(92, 142);
          this.eXang.Name = "eXang";
          this.eXang.Size = new System.Drawing.Size(202, 20);
          this.eXang.TabIndex = 5;
          this.eXang.Tag = "0";
          this.eXang.Text = "0.0";
          this.eXang.Validated += new System.EventHandler(this.eXang_Validated);
          this.eXang.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eZPos
          // 
          this.eZPos.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eZPos.Enabled = false;
          this.eZPos.Location = new System.Drawing.Point(92, 102);
          this.eZPos.Name = "eZPos";
          this.eZPos.Size = new System.Drawing.Size(202, 20);
          this.eZPos.TabIndex = 4;
          this.eZPos.Tag = "2";
          this.eZPos.Text = "0.0";
          this.eZPos.Validated += new System.EventHandler(this.eZPos_Validated);
          this.eZPos.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eYPos
          // 
          this.eYPos.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eYPos.Enabled = false;
          this.eYPos.Location = new System.Drawing.Point(92, 76);
          this.eYPos.Name = "eYPos";
          this.eYPos.Size = new System.Drawing.Size(202, 20);
          this.eYPos.TabIndex = 3;
          this.eYPos.Tag = "1";
          this.eYPos.Text = "0.0";
          this.eYPos.Validated += new System.EventHandler(this.eYPos_Validated);
          this.eYPos.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eXPos
          // 
          this.eXPos.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eXPos.Enabled = false;
          this.eXPos.Location = new System.Drawing.Point(92, 50);
          this.eXPos.Name = "eXPos";
          this.eXPos.Size = new System.Drawing.Size(202, 20);
          this.eXPos.TabIndex = 2;
          this.eXPos.Tag = "0";
          this.eXPos.Text = "0.0";
          this.eXPos.Validated += new System.EventHandler(this.eXPos_Validated);
          this.eXPos.Validating += new System.ComponentModel.CancelEventHandler(this.eFLoatValidating);
          // 
          // eTimeStamp
          // 
          this.eTimeStamp.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                      | System.Windows.Forms.AnchorStyles.Right)));
          this.eTimeStamp.Enabled = false;
          this.eTimeStamp.Location = new System.Drawing.Point(92, 15);
          this.eTimeStamp.Name = "eTimeStamp";
          this.eTimeStamp.Size = new System.Drawing.Size(202, 20);
          this.eTimeStamp.TabIndex = 1;
          // 
          // label8
          // 
          this.label8.AutoSize = true;
          this.label8.Location = new System.Drawing.Point(10, 236);
          this.label8.Name = "label8";
          this.label8.Size = new System.Drawing.Size(47, 13);
          this.label8.TabIndex = 7;
          this.label8.Text = "Rotation";
          // 
          // label7
          // 
          this.label7.AutoSize = true;
          this.label7.Location = new System.Drawing.Point(10, 197);
          this.label7.Name = "label7";
          this.label7.Size = new System.Drawing.Size(44, 13);
          this.label7.TabIndex = 6;
          this.label7.Text = "Azimuth";
          // 
          // label6
          // 
          this.label6.AutoSize = true;
          this.label6.Location = new System.Drawing.Point(10, 171);
          this.label6.Name = "label6";
          this.label6.Size = new System.Drawing.Size(51, 13);
          this.label6.TabIndex = 5;
          this.label6.Text = "Elevation";
          // 
          // label5
          // 
          this.label5.AutoSize = true;
          this.label5.Location = new System.Drawing.Point(9, 145);
          this.label5.Name = "label5";
          this.label5.Size = new System.Drawing.Size(25, 13);
          this.label5.TabIndex = 4;
          this.label5.Text = "Roll";
          // 
          // label4
          // 
          this.label4.AutoSize = true;
          this.label4.Location = new System.Drawing.Point(9, 105);
          this.label4.Name = "label4";
          this.label4.Size = new System.Drawing.Size(52, 13);
          this.label4.TabIndex = 3;
          this.label4.Text = "z-Position";
          // 
          // label3
          // 
          this.label3.AutoSize = true;
          this.label3.Location = new System.Drawing.Point(9, 79);
          this.label3.Name = "label3";
          this.label3.Size = new System.Drawing.Size(52, 13);
          this.label3.TabIndex = 2;
          this.label3.Text = "y-Position";
          // 
          // label2
          // 
          this.label2.AutoSize = true;
          this.label2.Location = new System.Drawing.Point(9, 50);
          this.label2.Name = "label2";
          this.label2.Size = new System.Drawing.Size(52, 13);
          this.label2.TabIndex = 1;
          this.label2.Text = "x-Position";
          // 
          // label1
          // 
          this.label1.AutoSize = true;
          this.label1.BackColor = System.Drawing.SystemColors.Control;
          this.label1.Location = new System.Drawing.Point(9, 22);
          this.label1.Name = "label1";
          this.label1.Size = new System.Drawing.Size(61, 13);
          this.label1.TabIndex = 0;
          this.label1.Text = "Time stamp";
          // 
          // gbSettings
          // 
          this.gbSettings.Controls.Add(this.udPort);
          this.gbSettings.Controls.Add(this.udMRate);
          this.gbSettings.Controls.Add(this.cbHemisphere);
          this.gbSettings.Controls.Add(this.cbBaudRate);
          this.gbSettings.Controls.Add(this.cbComPort);
          this.gbSettings.Controls.Add(this.btnApply);
          this.gbSettings.Controls.Add(this.eIPAddress);
          this.gbSettings.Controls.Add(this.label15);
          this.gbSettings.Controls.Add(this.label14);
          this.gbSettings.Controls.Add(this.label13);
          this.gbSettings.Controls.Add(this.label12);
          this.gbSettings.Controls.Add(this.label11);
          this.gbSettings.Controls.Add(this.label10);
          this.gbSettings.Dock = System.Windows.Forms.DockStyle.Fill;
          this.gbSettings.Location = new System.Drawing.Point(300, 3);
          this.gbSettings.MinimumSize = new System.Drawing.Size(300, 0);
          this.gbSettings.Name = "gbSettings";
          this.gbSettings.Size = new System.Drawing.Size(300, 441);
          this.gbSettings.TabIndex = 4;
          this.gbSettings.TabStop = false;
          this.gbSettings.Text = "Settings";
          // 
          // udPort
          // 
          this.udPort.Location = new System.Drawing.Point(98, 146);
          this.udPort.Maximum = new decimal(new int[] {
            35535,
            0,
            0,
            0});
          this.udPort.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
          this.udPort.Name = "udPort";
          this.udPort.Size = new System.Drawing.Size(191, 20);
          this.udPort.TabIndex = 25;
          this.udPort.Value = new decimal(new int[] {
            1234,
            0,
            0,
            0});
          // 
          // udMRate
          // 
          this.udMRate.Location = new System.Drawing.Point(97, 68);
          this.udMRate.Maximum = new decimal(new int[] {
            200,
            0,
            0,
            0});
          this.udMRate.Name = "udMRate";
          this.udMRate.Size = new System.Drawing.Size(192, 20);
          this.udMRate.TabIndex = 22;
          this.udMRate.Value = new decimal(new int[] {
            103,
            0,
            0,
            0});
          // 
          // cbHemisphere
          // 
          this.cbHemisphere.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbHemisphere.FormattingEnabled = true;
          this.cbHemisphere.Location = new System.Drawing.Point(97, 92);
          this.cbHemisphere.Name = "cbHemisphere";
          this.cbHemisphere.Size = new System.Drawing.Size(192, 21);
          this.cbHemisphere.TabIndex = 23;
          // 
          // cbBaudRate
          // 
          this.cbBaudRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbBaudRate.FormattingEnabled = true;
          this.cbBaudRate.Items.AddRange(new object[] {
            "110",
            "300",
            "600",
            "1200",
            "2400",
            "4800",
            "9600",
            "14400",
            "19200",
            "38400",
            "57600",
            "115200"});
          this.cbBaudRate.Location = new System.Drawing.Point(97, 41);
          this.cbBaudRate.MaxDropDownItems = 12;
          this.cbBaudRate.Name = "cbBaudRate";
          this.cbBaudRate.Size = new System.Drawing.Size(192, 21);
          this.cbBaudRate.TabIndex = 21;
          // 
          // cbComPort
          // 
          this.cbComPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
          this.cbComPort.FormattingEnabled = true;
          this.cbComPort.Items.AddRange(new object[] {
            "1",
            "2",
            "3",
            "4"});
          this.cbComPort.Location = new System.Drawing.Point(97, 15);
          this.cbComPort.Name = "cbComPort";
          this.cbComPort.Size = new System.Drawing.Size(192, 21);
          this.cbComPort.TabIndex = 20;
          // 
          // btnApply
          // 
          this.btnApply.Location = new System.Drawing.Point(214, 172);
          this.btnApply.Name = "btnApply";
          this.btnApply.Size = new System.Drawing.Size(75, 23);
          this.btnApply.TabIndex = 26;
          this.btnApply.Text = "Apply";
          this.btnApply.UseVisualStyleBackColor = true;
          this.btnApply.Click += new System.EventHandler(this.btnApply_Click);
          // 
          // eIPAddress
          // 
          this.eIPAddress.Location = new System.Drawing.Point(98, 119);
          this.eIPAddress.Name = "eIPAddress";
          this.eIPAddress.Size = new System.Drawing.Size(193, 20);
          this.eIPAddress.TabIndex = 24;
          this.eIPAddress.Text = "127.0.0.1";
          // 
          // label15
          // 
          this.label15.AutoSize = true;
          this.label15.Location = new System.Drawing.Point(7, 148);
          this.label15.Name = "label15";
          this.label15.Size = new System.Drawing.Size(52, 13);
          this.label15.TabIndex = 5;
          this.label15.Text = "UDP Port";
          // 
          // label14
          // 
          this.label14.AutoSize = true;
          this.label14.Location = new System.Drawing.Point(6, 122);
          this.label14.Name = "label14";
          this.label14.Size = new System.Drawing.Size(58, 13);
          this.label14.TabIndex = 4;
          this.label14.Text = "IP Address";
          // 
          // label13
          // 
          this.label13.AutoSize = true;
          this.label13.Location = new System.Drawing.Point(6, 96);
          this.label13.Name = "label13";
          this.label13.Size = new System.Drawing.Size(63, 13);
          this.label13.TabIndex = 3;
          this.label13.Text = "Hemisphere";
          // 
          // label12
          // 
          this.label12.AutoSize = true;
          this.label12.Location = new System.Drawing.Point(6, 70);
          this.label12.Name = "label12";
          this.label12.Size = new System.Drawing.Size(86, 13);
          this.label12.TabIndex = 2;
          this.label12.Text = "Measurment rate";
          // 
          // label11
          // 
          this.label11.AutoSize = true;
          this.label11.Location = new System.Drawing.Point(6, 44);
          this.label11.Name = "label11";
          this.label11.Size = new System.Drawing.Size(53, 13);
          this.label11.TabIndex = 1;
          this.label11.Text = "Baud rate";
          // 
          // label10
          // 
          this.label10.AutoSize = true;
          this.label10.Location = new System.Drawing.Point(6, 18);
          this.label10.Name = "label10";
          this.label10.Size = new System.Drawing.Size(50, 13);
          this.label10.TabIndex = 0;
          this.label10.Text = "Com Port";
          // 
          // GUI
          // 
          this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
          this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
          this.ClientSize = new System.Drawing.Size(602, 493);
          this.Controls.Add(this.tableLayoutPanel2);
          this.Controls.Add(this.statusStrip);
          this.Controls.Add(this.menuStrip);
          this.MainMenuStrip = this.menuStrip;
          this.MinimumSize = new System.Drawing.Size(610, 520);
          this.Name = "GUI";
          this.Text = "Flock of Birds Tracker";
          this.Load += new System.EventHandler(this.GUI_Load);
          this.menuStrip.ResumeLayout(false);
          this.menuStrip.PerformLayout();
          this.statusStrip.ResumeLayout(false);
          this.statusStrip.PerformLayout();
          this.tableLayoutPanel2.ResumeLayout(false);
          this.gbData.ResumeLayout(false);
          this.gbData.PerformLayout();
          this.tableLayoutPanel3.ResumeLayout(false);
          this.tableLayoutPanel3.PerformLayout();
          this.gbSettings.ResumeLayout(false);
          this.gbSettings.PerformLayout();
          ((System.ComponentModel.ISupportInitialize)(this.udPort)).EndInit();
          ((System.ComponentModel.ISupportInitialize)(this.udMRate)).EndInit();
          this.ResumeLayout(false);
          this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip;
      private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
      private System.Windows.Forms.StatusStrip statusStrip;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
      private System.Windows.Forms.TextBox textBox1;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
      private System.Windows.Forms.GroupBox gbData;
      private System.Windows.Forms.TextBox eZang;
      private System.Windows.Forms.TextBox eYang;
      private System.Windows.Forms.TextBox eXang;
      private System.Windows.Forms.TextBox eZPos;
      private System.Windows.Forms.TextBox eYPos;
      private System.Windows.Forms.TextBox eXPos;
      private System.Windows.Forms.TextBox eTimeStamp;
      private System.Windows.Forms.Label label8;
      private System.Windows.Forms.Label label7;
      private System.Windows.Forms.Label label6;
      private System.Windows.Forms.Label label5;
      private System.Windows.Forms.Label label4;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
      private System.Windows.Forms.TextBox eRot11;
      private System.Windows.Forms.TextBox eRot12;
      private System.Windows.Forms.TextBox eRot13;
      private System.Windows.Forms.TextBox eRot21;
      private System.Windows.Forms.TextBox eRot22;
      private System.Windows.Forms.TextBox eRot23;
      private System.Windows.Forms.TextBox eRot31;
      private System.Windows.Forms.TextBox eRot32;
      private System.Windows.Forms.TextBox eRot33;
      private System.Windows.Forms.Button btnMute;
      private System.Windows.Forms.Button btnPause;
      private System.Windows.Forms.Button btnSend;
      private System.Windows.Forms.GroupBox gbSettings;
      private System.Windows.Forms.Button btnApply;
      private System.Windows.Forms.TextBox eIPAddress;
      private System.Windows.Forms.Label label15;
      private System.Windows.Forms.Label label14;
      private System.Windows.Forms.Label label13;
      private System.Windows.Forms.Label label12;
      private System.Windows.Forms.Label label11;
      private System.Windows.Forms.Label label10;
      private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
      private System.Windows.Forms.ComboBox cbHemisphere;
      private System.Windows.Forms.ComboBox cbBaudRate;
      private System.Windows.Forms.ComboBox cbComPort;
      private System.Windows.Forms.NumericUpDown udPort;
      private System.Windows.Forms.NumericUpDown udMRate;
      private System.Windows.Forms.Panel pPreview;
    }
}

