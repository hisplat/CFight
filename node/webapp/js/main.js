
function get_request(name, default_var) {
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
    var r = window.location.search.substr(1).match(reg);
    if (r != null)
        return unescape(r[2]);
    return default_var;
}

function get_url_path() {
    url = window.location.href;
    var url_arr = [];
    url_arr = url.split('?');
    return url_arr[0];
}

function __ajax(action, data, success, fail, message_on_success) {
    $.ajax({
        url: "ajax.php?action=" + action,
        type: 'post',
        data: data,
        success: function (data) {
            console.debug(data);
            data = eval("(" + data + ")");
            if (data.ret == "success") {
                if (typeof(message_on_success) == "string") {
                    alert(message_on_success);
                } else if (typeof(data.reason) != "undefined") {
                    alert(data.reason);
                }

                if (typeof(success) == "string") {
                    document.location.href = success;
                } else if (typeof(success) == "function") {
                    success(data);
                } else if (typeof(success) == "boolean") {
                    if (success) {
                        document.location.reload();
                    }
                } else {
                }
            } else {
                if (typeof(data.reason) == 'undefined') {
                    // alert(data.ret);
                } else {
                    alert(data.reason);
                }
                if (typeof(fail) == "string") {
                    document.location.href = fail;
                } else if (typeof(fail) == "function") {
                    fail(data);
                } else if (typeof(fail) == "boolean") {
                    if (fail) {
                        document.location.reload();
                    }
                } else {
                }
            }
        },
        error: function(object, info) {
            if (info == "timeout") {
                alert("服务器连接超时，请稍后再试。");
            } else if (info == "notmodified") {
            } else if (info == "error") {
            } else if (info == "parsererror") {
            } else {
            }

            if (typeof(fail) == "string") {
                document.location.href = fail;
            } else if (typeof(fail) == "function") {
                fail(info);
            } else if (typeof(fail) == "boolean") {
                if (fail) {
                    document.location.reload();
                }
            } else {
            }
        }
    });
}

function __request(action, data, success, fail) {
    $.ajax({
        url: "index.php?action=" + action,
        type: 'post',
        data: data,
        success: function (data) {
            if (!data.startsWith("{")) {
                console.debug(data);
                return;
            }
            data = eval("(" + data + ")");
            if (typeof(success) == "string") {
                document.location.href = success;
            } else if (typeof(success) == "function") {
                success(data);
            } else if (typeof(success) == "boolean") {
                if (success) {
                    document.location.reload();
                }
            } else {
                console.debug(typeof(success));
            }
        },
        error: function(object, info) {
            if (info == "timeout") {
                alert("服务器连接超时，请稍后再试。");
            } else if (info == "notmodified") {
            } else if (info == "error") {
            } else if (info == "parsererror") {
            } else {
            }

            if (typeof(fail) == "string") {
                document.location.href = fail;
            } else if (typeof(fail) == "function") {
                fail(info);
            } else if (typeof(fail) == "boolean") {
                if (fail) {
                    document.location.reload();
                }
            } else {
            }
        }
    });
}

function __ajax_and_reload(action, data, message_on_success) {
    __ajax(action, data, true, false, message_on_success);
}

function __file_upload(action, obj, arg, success, fail) {
    if (typeof FileReader == 'undefined') {
        alert("您的浏览器不支持上传，请更换浏览器重试！");
        return;
    }

    var file = obj.files[0];
    var reader = new FileReader();
    var onload = function(e) {
        console.debug(e);
        var data = e.target.result;
        __request(action, {arg: arg, data: data}, success, fail);
    };

    reader.onload = onload;
    console.debug("upload file " + file + " to " + action);
    reader.readAsDataURL(file);
}

function __image_upload(obj, success, fail, extra) {
    if (typeof FileReader == 'undefined') {
        alert("您的浏览器不支持上传，请更换浏览器重试！");
        return;
    }

    var file = obj.files[0];
    var reader = new FileReader();
    var action = $(obj).attr("action");
    reader.onload = function(e) {
        var imgsrc = e.target.result;
        // console.debug(imgsrc);
        __ajax(action, {imgsrc: imgsrc, extra: extra}, success, fail);
    }
    console.debug("upload file " + file + " to " + action);
    reader.readAsDataURL(file);
}





String.prototype.trim = function() {
    return this.replace(/(^\s*)|(\s*$)/g, "");
}


if (typeof(String.prototype.startsWith) != 'function') {
    String.prototype.startsWith = function(prefix) {
        return this.slice(0, prefix.length) === prefix;
    };
}

