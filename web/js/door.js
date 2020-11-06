
function openTheDoor() {
  var token = $.cookie('token');
  if (token == null || token == '') {
    return layer.alert('请先设置Token');
  }
  var url = 'https://aa.yy.zz/door/open?from=web'; 
  url += '&token=' + token;
  $.ajax({
    url: url,
    type:'get',
    success:function(res){
      if (!("code" in res) || !("msg" in res)) {
        return layer.alert('服务器内部错误');
      }
      if (res.code != 0) {
        return layer.alert(res.msg);
      }
    },
    error:function(XMLHttpRequest, textStatus, errorThrown) {
      layer.alert('请求失败，请稍后重试');
    }
  });
}

function onSetting() {
  layer.prompt({
    formType: 0,
    value: '',
    shade: 0.3,
    shadeClose: true,
    title: '请输入Token：',
  }, function(token, index, elem){
    if (token != null && token != '') {
      $.cookie('token', token, { expires: 365, domain: 'bb.yy.zz', path: '/' });
    }
    layer.close(index);
  });
}

function copyToClipboard(content) {
  if (content == null || content == '') {
    return;
  }
  var inputEle = document.createElement('input')
  document.body.appendChild(inputEle)
  inputEle.setAttribute('value', content)
  inputEle.setAttribute('readonly', 'readonly')
  inputEle.focus()
  inputEle.setSelectionRange(0, inputEle.value.length)
  inputEle.select()
  document.execCommand('copy')
  document.body.removeChild(inputEle)
}

function genTempLink() {
  var token = $.cookie('token');
  if (token == null || token == '') {
    return layer.alert('请先设置Token');
  }
  layer.open({
    type: 2,
    title: "临时开门链接",
    area: ['300px', '270px'],
    shade: 0.3,
    shadeClose: true,
    content: 'http://bb.yy.zz/new-link-form.html',
    btn:['生成'],
    yes:function(index, layero){
      var body = top.layer.getChildFrame('body', index);
      var iframeWin = window[layero.find('iframe')[0]['name']];
      var user = body.find('#user').val();
      var max_times = body.find('#max-times').val();
      var ttl = body.find('#ttl').val();
      if (user == '' || max_times == '' || ttl == '') {
        return layer.alert('请把表格填写完整');
      }
      /* token */
      var token = $.cookie('token');
      if (token == null || token == '') {
        return layer.alert('请先设置Token');
      }
      /* url */
      var url = 'https://aa.yy.zz/token/new?'; 
      url += 'token=' + token;
      url += '&user=' + user;
      url += '&max_times=' + max_times;
      url += '&ttl=' + ttl;
      /* GET request */
      $.ajax({
        url: url,
        type:'get',
        success:function(res){
          if (!("code" in res) || !("msg" in res)) {
            return layer.alert('服务器内部错误');
          }
          if (res.code != 0) {
            return layer.alert(res.msg);
          }
          if (!("data" in res) || !("token" in res.data)) {
            return layer.alert('服务器内部错误');
          }
          console.log(res.data.token);
          var tempLink = 'https://aa.yy.zz/door/open?from=web';
          tempLink += '&token=' + res.data.token;
          copyToClipboard(tempLink);
          layer.alert('链接已复制到剪贴板',{
            time: 2000,
            end: function() {
              layer.closeAll();
            }
          });
        },
        error:function(res) {
          layer.alert('请求失败，请稍后重试');
        }
      });
    },
    closeBtn : 1,
    btnAlign:'c',
    success : function(layero, index) {
    },
    cancel: function(layero, index) {
    },
    end: function() {
      layer.closeAll();
    }
  });
}

