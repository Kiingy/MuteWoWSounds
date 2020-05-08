namespace MuteWowSounds {
    partial class MuteSoundsUI {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.RunButton = new System.Windows.Forms.Button();
            this.versionLabel = new System.Windows.Forms.Label();
            this.ExitBtn = new System.Windows.Forms.Button();
            this.RunError = new System.Windows.Forms.Label();
            this.TextBoxLog = new MuteWowSounds.ReadOnlyRichTextBox();
            this.ClearLog = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // RunButton
            // 
            this.RunButton.Location = new System.Drawing.Point(10, 10);
            this.RunButton.Name = "RunButton";
            this.RunButton.Size = new System.Drawing.Size(75, 23);
            this.RunButton.TabIndex = 1;
            this.RunButton.Text = "Start";
            this.RunButton.UseVisualStyleBackColor = true;
            this.RunButton.Click += new System.EventHandler(this.RunButton_Click);
            // 
            // versionLabel
            // 
            this.versionLabel.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.versionLabel.ForeColor = System.Drawing.Color.Black;
            this.versionLabel.Location = new System.Drawing.Point(170, 10);
            this.versionLabel.Name = "versionLabel";
            this.versionLabel.Size = new System.Drawing.Size(167, 23);
            this.versionLabel.TabIndex = 2;
            this.versionLabel.Text = "v1.0 - 15/04/20";
            this.versionLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // ExitBtn
            // 
            this.ExitBtn.Location = new System.Drawing.Point(91, 10);
            this.ExitBtn.Name = "ExitBtn";
            this.ExitBtn.Size = new System.Drawing.Size(73, 23);
            this.ExitBtn.TabIndex = 3;
            this.ExitBtn.Text = "Exit";
            this.ExitBtn.UseVisualStyleBackColor = true;
            this.ExitBtn.Click += new System.EventHandler(this.ExitBtn_Click);
            // 
            // RunError
            // 
            this.RunError.ForeColor = System.Drawing.Color.Red;
            this.RunError.Location = new System.Drawing.Point(10, 230);
            this.RunError.Name = "RunError";
            this.RunError.Size = new System.Drawing.Size(440, 20);
            this.RunError.TabIndex = 4;
            this.RunError.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // TextBoxLog
            // 
            this.TextBoxLog.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.TextBoxLog.Font = new System.Drawing.Font("Times New Roman", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.TextBoxLog.Location = new System.Drawing.Point(10, 40);
            this.TextBoxLog.Name = "TextBoxLog";
            this.TextBoxLog.Size = new System.Drawing.Size(372, 199);
            this.TextBoxLog.TabIndex = 5;
            this.TextBoxLog.Text = "";
            // 
            // ClearLog
            // 
            this.ClearLog.Location = new System.Drawing.Point(343, 10);
            this.ClearLog.Name = "ClearLog";
            this.ClearLog.Size = new System.Drawing.Size(39, 25);
            this.ClearLog.TabIndex = 6;
            this.ClearLog.Text = "Clear";
            this.ClearLog.UseVisualStyleBackColor = true;
            this.ClearLog.Click += new System.EventHandler(this.ClearLog_Click);
            // 
            // MuteSoundsUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(394, 251);
            this.Controls.Add(this.ClearLog);
            this.Controls.Add(this.TextBoxLog);
            this.Controls.Add(this.RunError);
            this.Controls.Add(this.ExitBtn);
            this.Controls.Add(this.versionLabel);
            this.Controls.Add(this.RunButton);
            this.MaximumSize = new System.Drawing.Size(410, 290);
            this.MinimumSize = new System.Drawing.Size(360, 290);
            this.Name = "MuteSoundsUI";
            this.Text = "Mute Annoying WoW Sounds";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button RunButton;
        private System.Windows.Forms.Label versionLabel;
        private System.Windows.Forms.Button ExitBtn;
        private System.Windows.Forms.Label RunError;
        private ReadOnlyRichTextBox TextBoxLog;
        private System.Windows.Forms.Button ClearLog;
    }
}

