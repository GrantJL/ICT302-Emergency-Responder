
// Get Titan interfaces
var $events = $eview.$query_interface("ti::js::TitanEventInterface.instance");

var Settings = 
{
    "radius": 2,
    "damage": 0.2
}

var EmergencyResponder = 
{

    /**
    * Initialize the plugin
    */
    init:function()
    {
        // Set window size and position on screen
		window.resizeTo(300, 460);
		window.moveTo(0, 100);

        // Request Entities button
        $('button[name=reqEnts]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::request", null);
            }
        });


        $('#radius').change(function() {
            $('#radiusT').val($('#radius').val());
            Settings.radius = parseFloat($('#radius').val());
            $events.sendEventArgs("EResp::Settings", Settings);
        });
        $('#damage').change(function() {
            $('#damageT').val($('#damage').val());
            Settings.damage = parseFloat($('#damage').val());
            $events.sendEventArgs("EResp::Settings", Settings);
        });

        // Request Entities button
        $('button[name=initFire]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::Begin", null);
            }
        });

        // Register event handler for incoming events
        $global.titanEventHandlers["EResp::entities"] = EmergencyResponder.updateEntities;

        $global.fadingLog( "EmergencyResponder plugin loaded!" );
    },

    updateEntities:function(name, args)
    {
        var fires = args.fires; // Get array at key "entities"
        var txt = "Fires<ul class='list'>"; // Start a list for innerHTML
        
        // For each obj(String) in names array
        $.each(fires, function(i, v){
            txt += "<li>" + v.name + " [" + v.id + "]</li>";
        })
        
        txt += "</ul>";

        var buildings = args.buildings;

        txt += "Buildings<ul class='list'>"; // Start a list for innerHTML
        
        // For each obj(String) in names array
        $.each(buildings, function(i, v){
            txt += "<li>" + v.name + "</li>";
        })
        
        txt += "</ul>";

        
        // Get args json as a string
        var json = JSON.stringify(args);

        $("#entityList").html(txt); // Display entity names in a list
        $("#test").text(json);      // Output json string to Testing/Debug tab
    }
}

// Initialize our plugin object
window.addEventListener('DOMContentLoaded', EmergencyResponder.init);