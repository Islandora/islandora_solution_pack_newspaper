(function($) {
Drupal.behaviors.loadNewspaperBehavior = {
  attach: function (context, settings) {

    $(".islandora-newspaper-content ul .vertical-tab-button a").click(function() {
      // From the current link, get the year value from the text.
      var a = $(this);
      var year = a.first()[0].innerText
      year = year.replace('(active tab)', '').trim();

      // Also, need to grab the hidden PID field value.
      var pid = $('#hidden-pid').text().trim();

      if (year && pid) {
        newspaper_year_callback(pid, year);
      }
    });

  }
};
})(jQuery);


function newspaper_year_callback(pid, year) {
  if (pid && year) {
    var url = window.location.protocol + "//" + window.location.host + "/islandora/object/" + pid + "/newspaper/get_year/" + year;
    ajax=islandora_newspaper_AjaxCaller(); 
    ajax.open("GET", url, true); 
    ajax.onreadystatechange=function(){
        if(ajax.readyState==4){
            if(ajax.status==200){
                jQuery("#y_" + year).html(ajax.responseText);
            }
        }
    }
    ajax.send(null);
  }
}

function islandora_newspaper_AjaxCaller(){
  var xmlhttp=false;
  try{
    xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
  }catch(e){
    try{
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }catch(E){
      xmlhttp = false;
    }
  }

  if(!xmlhttp && typeof XMLHttpRequest!='undefined'){
    xmlhttp = new XMLHttpRequest();
  }
  return xmlhttp;
}
