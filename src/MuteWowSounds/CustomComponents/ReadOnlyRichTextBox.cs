using System;
using System.Windows.Forms;
using System.ComponentModel;
using System.Runtime.InteropServices;

namespace MuteWowSounds {
    public class ReadOnlyRichTextBox : System.Windows.Forms.RichTextBox {

        [DllImport("user32.dll")]
        private static extern int HideCaret(IntPtr hwnd);

        public ReadOnlyRichTextBox() {
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.ReadOnlyRichTextBox_Mouse);
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.ReadOnlyRichTextBox_Mouse);
            base.ReadOnly = true;
            base.TabStop = false;
            HideCaret(this.Handle);
        }


        protected override void OnGotFocus(EventArgs e) {
            HideCaret(this.Handle);
        }

        protected override void OnEnter(EventArgs e) {
            HideCaret(this.Handle);
        }

        [DefaultValue(true)]
        public new bool ReadOnly {
            get { return true; }
            set { }
        }

        [DefaultValue(false)]
        public new bool TabStop {
            get { return false; }
            set { }
        }

        private void ReadOnlyRichTextBox_Mouse(object sender, System.Windows.Forms.MouseEventArgs e) {
            HideCaret(this.Handle);
        }

    }
}
