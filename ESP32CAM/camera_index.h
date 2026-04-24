const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Camera</title>
</head>

<body style="background:#111;color:white;text-align:center;font-family:sans-serif">

<h1>Camera System</h1>

<button onclick="capture()" style="padding:12px 24px;font-size:18px;">
Capture Vehicle
</button>

<br><br>

<img id="img" width="320" style="border:3px solid white">

<hr>

<h2>History</h2>
<div id="history"></div>

<script>

let history = JSON.parse(localStorage.getItem("history")) || [];

function capture(){
  const imgUrl = "/capture?" + new Date().getTime();
  document.getElementById("img").src = imgUrl;

  const entry = {
    img: imgUrl,
    time: new Date().toLocaleTimeString()
  };

  history.unshift(entry);
  localStorage.setItem("history", JSON.stringify(history));

  render();
}

function render(){
  let html = "";
  history.forEach(h=>{
    html += `
      <div style="border:1px solid white;margin:10px;padding:10px">
        <img src="${h.img}" width="200"><br>
        ${h.time}
      </div>
    `;
  });
  document.getElementById("history").innerHTML = html;
}

render();

</script>

</body>
</html>
)rawliteral";