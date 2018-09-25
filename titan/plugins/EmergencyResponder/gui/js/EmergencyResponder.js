
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
		window.resizeTo(275, 260);
		window.moveTo(0, 100);

        // Add button event handlers
        $('button.rotate').on(
        {
            'click':function()
            {
                var button = $(this);
                var params = button.data('params'); // parameters from html button
                $events.sendEventArgs("EResp::rotate", params); // send to event bus
            }
        });

        // Request Entities button
        $('button[name=reqEnts]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::request", null);
            }
        });

        // Request Entities button
        $('button[name=spawnFire]').on(
        {
            'click':function()
            {
                $events.sendEventArgs("EResp::spawnFire", null);
                var descriptor = $world.getEntityDescriptorFromName("er_wildfire_system");
                var fire = $scenario.createEntityECEF(descriptor, posTo);
                fire.snapToGround();
            }
        });

        // Register event handler for incoming events
        $global.titanEventHandlers["EResp::entities"] = EmergencyResponder.updateEntities;

        $global.fadingLog( "EmergencyResponder plugin loaded!" );
    },

    updateEntities:function(name, args)
    {
        var names = args.entities; // Get array at key "entities"
        var txt = "<ul>"; // Start a list for innerHTML
        
        // For each obj(String) in names array
        $.each(names, function(i, v){
            txt += "<li>" + v + "</li>";
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