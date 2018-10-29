
// Get Titan interfaces
var $events = $eview.$query_interface("ti::js::TitanEventInterface.instance");

var EmergencyResponder = 
{

    /**
    * Initialize the plugin
    */
    init:function()
    {
        // Set window size and position on screen
		window.resizeTo(275, 460);
		window.moveTo(0, 100);

        // Request Entities button
        $('button[name=reqEnts]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::request", null);
            }
        });

        // Request Entities button
        $('button[name=initFire]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::Begin", null);
            }
        });

         // Request Entities button
         $('button[name=dmgReport]').on(
            {
                'click':function()
                {
                    $events.sendEventArgs("EResp::DamageReport", null);
                }
            });
        
        $('#dispatchFiretruck').on(
        {
            'click':function()
            {
                $('#dispatchFiretruck').text("Dispatching...");
                $('#dispatchFiretruck').attr("disabled", true);
                $('#firetrucksList').attr("disabled", true);
                $events.sendEventArgs("EResp::Dispatch", {uuid: $('#firetrucksList').val()});
            }
        });
        
        $('#firetrucksList').change(EmergencyResponder.firetruckListChanged).change();

        

        // Register event handler for incoming events
        $global.titanEventHandlers["EResp::entities"] = EmergencyResponder.updateEntities;
        $global.titanEventHandlers["EResp::SendFiretrucks"] = EmergencyResponder.updateFiretruckList;
        $global.titanEventHandlers["EResp::ResetUI"] = EmergencyResponder.resetUI;

        $events.sendEventArgs("EResp::RequestFiretrucks", null);

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
    },

    updateFiretruckList:function(name, args)
    {

        $("#firetrucksList").append(new Option(args.name, args.uuid));

    },

    firetruckListChanged:function()
    {
        if($('#firetrucksList').val() == "--")
        {
            $('#dispatchFiretruck').attr("disabled", true);
        }
        else
        {
            $('#dispatchFiretruck').removeAttr("disabled");
        }
    },

    resetUI:function()
    {
        $('#dispatchFiretruck').removeAttr("disabled");
        $('#dispatchFiretruck').text("Dispatch");
        $('#firetrucksList').removeAttr("disabled");
    }
}

// Initialize our plugin object
window.addEventListener('DOMContentLoaded', EmergencyResponder.init);