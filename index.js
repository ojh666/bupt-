


const devicesId = "575179617"
const api_key = "AsEh2aZWBXfEd52BpzRuHUTC1=c="
Page({
  next:function(event){
    wx.redirectTo({
      url:"../second/second",
      success:function(){
        console.log("ok")
      }
    });
  },
  data: {
    a:"",
    b:""
  },
  
  change: function () {
    var that = this;
    this.setData({
      a: "",
      b: "1"
    })
    
  },

  getOne: function () {
  
    this.getDatapoints().then(res => {
      this.setData({
        a: res.light[19].value
        
      })
      if (res.light[19].value < 1 ) {
        this.setData({
          b: "7"
        })
      }
      else {
        this.setData({
          b: ""
        })
      }
    })
    

  },
  getDatapoints: function () {
    return new Promise((resolve, reject) => {
      wx.request({
        url: `https://api.heclouds.com/devices/${devicesId}/datapoints?datastream_id=Light,Temperature,Humidity&limit=20`,
        /**
         * 添加HTTP报文的请求头, 
         * 其中api-key为OneNet的api文档要求我们添加的鉴权秘钥
         * Content-Type的作用是标识请求体的格式, 从api文档中我们读到请求体是json格式的
         * 故content-type属性应设置为application/json
         */
        header: {
          'content-type': 'application/json',
          'api-key': api_key
        },
        success: (res) => {
          const status = res.statusCode
          const response = res.data

          // console.log(response.data.datastreams[0].datapoints[0].value)
          if (status !== 200) { // 返回状态码不为200时将Promise置为reject状态
            reject(res.data)
            return;
          }
          if (response.errno !== 0) { //errno不为零说明可能参数有误, 将Promise置为reject
            reject(response.error)
            return;
          }

          if (response.data.datastreams.length === 0) {
            reject("当前设备无数据, 请先运行硬件实验")
          }

          //程序可以运行到这里说明请求成功, 将Promise置为resolve状态
          resolve({
            temperature: response.data.datastreams[0].datapoints.reverse(),
            light: response.data.datastreams[1].datapoints.reverse(),
            humidity: response.data.datastreams[2].datapoints.reverse()
          })
        },
        fail: (err) => {
          reject(err)
        }
      })
    })
  },

  onLoad: function () {

    //每隔6s自动获取一次数据进行更新
    // const timer = setInterval(() => {
    //   this.getDatapoints().then(res => {
    //     this.setData({
    //       a: res.light[19].value

    //     })
    //     if (res.light[19].value < 1) {
    //       this.setData({
    //         b: "7"
    //       })
    //     }
    //     else {
    //       this.setData({
    //         b: ""
    //       })
    //     }
    //   })

    // }, 1000)

  



}
})




