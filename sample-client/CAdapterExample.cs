using System;
using System.Windows.Forms;
using WebSocket4Net;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace WindowsFormsApplication1
{
    public partial class CAdapterExample : Form
    {
        WebSocket websocket = new WebSocket("ws://127.0.0.1:7877", "lws-mirror-protocol");

        delegate void AddMessageCallback(string text);

        public CAdapterExample()
        {
            InitializeComponent();
        }

        private void CAdapterExample_Load(object sender, EventArgs e)
        {
            websocket.Opened += new EventHandler(websocket_Opened);
            websocket.MessageReceived += new EventHandler<MessageReceivedEventArgs>(websocket_MessageReceived);
            websocket.Open();
        }

        private void websocket_Opened(object sender, EventArgs e)
        {
            JObject obj = new JObject();
            obj["name"] = "connection-success";
            websocket.Send(obj.ToString());
        }

        private void websocket_MessageReceived(object sender, MessageReceivedEventArgs e)
        {
            JObject obj = JObject.Parse(e.Message);

            // Now do something with the parsed message
        }

        private void button_Click(object sender, EventArgs e)
        {
            JObject obj  = new JObject();
            obj["name"]  = "setScene";
            obj["value"] = "0";

            try
            {
                obj["value"] = Convert.ToInt32(textScene.Text);
            }
            catch (Exception)
            {
                // Do nothing
            }

            websocket.Send(obj.ToString());
        }
    }
}
