namespace WindowsFormsApplication1
{
    partial class CAdapterExample
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
            this.label1 = new System.Windows.Forms.Label();
            this.button = new System.Windows.Forms.Button();
            this.textScene = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(224, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Click the button to change the dang scene <3";
            // 
            // button
            // 
            this.button.Location = new System.Drawing.Point(85, 25);
            this.button.Name = "button";
            this.button.Size = new System.Drawing.Size(178, 23);
            this.button.TabIndex = 2;
            this.button.Text = "Change Scene <3";
            this.button.UseVisualStyleBackColor = true;
            this.button.Click += new System.EventHandler(this.button_Click);
            // 
            // textScene
            // 
            this.textScene.Location = new System.Drawing.Point(12, 25);
            this.textScene.Name = "textScene";
            this.textScene.Size = new System.Drawing.Size(67, 20);
            this.textScene.TabIndex = 3;
            // 
            // CAdapterExample
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(275, 61);
            this.Controls.Add(this.textScene);
            this.Controls.Add(this.button);
            this.Controls.Add(this.label1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CAdapterExample";
            this.Text = "Scene Changer (Sample Code)";
            this.Load += new System.EventHandler(this.CAdapterExample_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button button;
        private System.Windows.Forms.TextBox textScene;

    }
}

