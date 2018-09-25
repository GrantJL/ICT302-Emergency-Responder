/**

seedrandom.js
=============

Seeded random number generator for Javascript.

version 2.3.10
Author: David Bau
Date: 2014 Sep 20

**/

(function (
    global, pool, math, width, chunks, digits, module, define, rngname) {

//
// The following constants are related to IEEE 754 limits.
//
var startdenom = math.pow(width, chunks),
    significance = math.pow(2, digits),
    overflow = significance * 2,
    mask = width - 1,
    nodecrypto;

//
// seedrandom()
// This is the seedrandom function described above.
//
var impl = math['seed' + rngname] = function(seed, options, callback) {
  var key = [];
  options = (options == true) ? { entropy: true } : (options || {});

  // Flatten the seed string or build one from local entropy if needed.
  var shortseed = mixkey(flatten(
    options.entropy ? [seed, tostring(pool)] :
    (seed == null) ? autoseed() : seed, 3), key);

  // Use the seed to initialize an ARC4 generator.
  var arc4 = new ARC4(key);

  // Mix the randomness into accumulated entropy.
  mixkey(tostring(arc4.S), pool);

  // Calling convention: what to return as a function of prng, seed, is_math.
  return (options.pass || callback ||
      // If called as a method of Math (Math.seedrandom()), mutate Math.random
      // because that is how seedrandom.js has worked since v1.0.  Otherwise,
      // it is a newer calling convention, so return the prng directly.
      function(prng, seed, is_math_call) {
        if (is_math_call) { math[rngname] = prng; return seed; }
        else return prng;
      })(

  // This function returns a random double in [0, 1) that contains
  // randomness in every bit of the mantissa of the IEEE 754 value.
  function() {
    var n = arc4.g(chunks),             // Start with a numerator n < 2 ^ 48
        d = startdenom,                 //   and denominator d = 2 ^ 48.
        x = 0;                          //   and no 'extra last byte'.
    while (n < significance) {          // Fill up all significant digits by
      n = (n + x) * width;              //   shifting numerator and
      d *= width;                       //   denominator and generating a
      x = arc4.g(1);                    //   new least-significant-byte.
    }
    while (n >= overflow) {             // To avoid rounding up, before adding
      n /= 2;                           //   last byte, shift everything
      d /= 2;                           //   right using integer math until
      x >>>= 1;                         //   we have exactly the desired bits.
    }
    return (n + x) / d;                 // Form the number within [0, 1).
  }, shortseed, 'global' in options ? options.global : (this == math));
};

//
// ARC4
//
// An ARC4 implementation.  The constructor takes a key in the form of
// an array of at most (width) integers that should be 0 <= x < (width).
//
// The g(count) method returns a pseudorandom integer that concatenates
// the next (count) outputs from ARC4.  Its return value is a number x
// that is in the range 0 <= x < (width ^ count).
//
/** @constructor */
function ARC4(key) {
  var t, keylen = key.length,
      me = this, i = 0, j = me.i = me.j = 0, s = me.S = [];

  // The empty key [] is treated as [0].
  if (!keylen) { key = [keylen++]; }

  // Set up S using the standard key scheduling algorithm.
  while (i < width) {
    s[i] = i++;
  }
  for (i = 0; i < width; i++) {
    s[i] = s[j = mask & (j + key[i % keylen] + (t = s[i]))];
    s[j] = t;
  }

  // The "g" method returns the next (count) outputs as one number.
  (me.g = function(count) {
    // Using instance members instead of closure state nearly doubles speed.
    var t, r = 0,
        i = me.i, j = me.j, s = me.S;
    while (count--) {
      t = s[i = mask & (i + 1)];
      r = r * width + s[mask & ((s[i] = s[j = mask & (j + t)]) + (s[j] = t))];
    }
    me.i = i; me.j = j;
    return r;
    // For robust unpredictability, the function call below automatically
    // discards an initial batch of values.  This is called RC4-drop[256].
    // See http://google.com/search?q=rsa+fluhrer+response&btnI
  })(width);
}

//
// flatten()
// Converts an object tree to nested arrays of strings.
//
function flatten(obj, depth) {
  var result = [], typ = (typeof obj), prop;
  if (depth && typ == 'object') {
    for (prop in obj) {
      try { result.push(flatten(obj[prop], depth - 1)); } catch (e) {}
    }
  }
  return (result.length ? result : typ == 'string' ? obj : obj + '\0');
}

//
// mixkey()
// Mixes a string seed into a key that is an array of integers, and
// returns a shortened string seed that is equivalent to the result key.
//
function mixkey(seed, key) {
  var stringseed = seed + '', smear, j = 0;
  while (j < stringseed.length) {
    key[mask & j] =
      mask & ((smear ^= key[mask & j] * 19) + stringseed.charCodeAt(j++));
  }
  return tostring(key);
}

//
// autoseed()
// Returns an object for autoseeding, using window.crypto if available.
//
/** @param {Uint8Array|Navigator=} seed */
function autoseed(seed) {
  try {
    if (nodecrypto) return tostring(nodecrypto.randomBytes(width));
    global.crypto.getRandomValues(seed = new Uint8Array(width));
    return tostring(seed);
  } catch (e) {
    return [+new Date, global, (seed = global.navigator) && seed.plugins,
      global.screen, tostring(pool)];
  }
}

//
// tostring()
// Converts an array of charcodes to a string
//
function tostring(a) {
  return String.fromCharCode.apply(0, a);
}

//
// When seedrandom.js is loaded, we immediately mix a few bits
// from the built-in RNG into the entropy pool.  Because we do
// not want to interfere with deterministic PRNG state later,
// seedrandom will not call math.random on its own again after
// initialization.
//
mixkey(math[rngname](), pool);

//
// Nodejs and AMD support: export the implementation as a module using
// either convention.
//
if (module && module.exports) {
  module.exports = impl;
  try {
    // When in node.js, try using crypto package for autoseeding.
    nodecrypto = require('crypto');
  } catch (ex) {}
} else if (define && define.amd) {
  define(function() { return impl; });
}

//
// Node.js native crypto support.
//

// End anonymous scope, and pass initial values.
})(
  this,   // global window object
  [],     // pool: entropy pool starts empty
  Math,   // math: package containing random, pow, and seedrandom
  256,    // width: each RC4 output is 0 <= x < 256
  6,      // chunks: at least six RC4 outputs for each double
  52,     // digits: there are 52 significant digits in a double
  (typeof module) == 'object' && module,    // present in node.js
  (typeof define) == 'function' && define,  // present with an AMD loader
  'random'// rngname: name for Math.random and Math.seedrandom
);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var PI = Math.PI;

function clamp(val, min, max){if(val < min) return min;else if(val > max) return max;else return val;}

//degrees to radians
function deg2rad(angle_deg) { return (PI / 180.0) * angle_deg ;}

//radians to degrees
function rad2deg(angle_rad){ return angle_rad * 180.0 / PI ;}

//normalize input value of range r1 to output range r2
//normalizeRange(-1, [ -2.1, 0.55 ], [ 0, 1 ])
function normalizeRange( val, r1, r2 ) { 
  if( ( r1[ 1 ] - r1[ 0 ] ) != 0){
    return ( val - r1[ 0 ] ) * ( r2[ 1 ] - r2[ 0 ] ) / ( r1[ 1 ] - r1[ 0 ] ) + r2[ 0 ];
  } else {
    return val;
  }
}

// //HASTINGS.  MAX ERROR = .000001
// //The standard deviation must be nonnegative
// function normalcdf(Z, M, SD){
//   var X = (Z-M)/SD;
//   var T=1/(1+.2316419*Math.abs(X));
//   var D=.3989423*Math.exp(-X*X/2);
//   var Prob=D*T*(.3193815+T*(-.3565638+T*(1.781478+T*(-1.821256+T*1.330274))));
//   if (X>0) {
//     Prob=1-Prob
//   }
//   return Prob
// }

function normalcdf(mean, sigma, to) 
{
    var z = (to-mean)/Math.sqrt(2*sigma*sigma);
    var t = 1/(1+0.3275911*Math.abs(z));
    var a1 =  0.254829592;
    var a2 = -0.284496736;
    var a3 =  1.421413741;
    var a4 = -1.453152027;
    var a5 =  1.061405429;
    var erf = 1-(((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*Math.exp(-z*z);
    var sign = 1;
    if(z < 0)
    {
        sign = -1;
    }
    return (1/2)*(1+sign*erf);
}

normalcdf(30, 25, 1.4241); //-> 0.000223264606750539
//wolframalpha.com              0.000223221102572082

//Pseudorandom number generator
function getPRNG(){
    Math.seedrandom();
    return Math.random();
}

function getRandomRange(range){
    return normalizeRange(getPRNG(), [ 0, 1 ], [ -range, range ]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////
    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    
////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////    ////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var $sketch = $world.$query_interface('ot::js::sketch.get');
var sketchFireVisualization = false;
var fireProgression;
var sketch_blue   = 0xFFFFFF00;
var sketch_green  = 0xFF00FF00;
var sketch_red    = 0xFF0000FF;
var sketch_yellow = 0xFF00FFFF;
var sketch_white  = 0xFFFFFFFF;
var sketch_black  = 0xFF000000;
var sketch_gold   = 0xFF1FF8FF;

function createFireSketch(){
  {
    fireProgression = $sketch.create_group();
    $sketch.make_group_active(fireProgression);
    $sketch.set_xray_mode(true);
    $console.error_log("CREATE SKETCH GROUP!");
  }
}

function deleteFireSketch(){
  {
    $sketch.delete_group(fireProgression);
  }
}

function sketchFirePropagation(pos1, pos2, intensity){
  {
    var color = sketch_blue;
    if(intensity == 0){ color = sketch_blue; }
    if(intensity == 1){ color = sketch_green; }
    if(intensity == 2){ color = sketch_yellow; }
    if(intensity == 3){ color = sketch_red; }

    $sketch.set_color(color);
    $sketch.set_hidden_color(color);

    $sketch.draw_line_ecef(pos1, true);
    $sketch.draw_line_ecef(pos2, false);
    $console.error_log("SKETCH!");
  }
}

var firePos, fireHdg, fireEvents;

var fireNum = 8;
var fireMaxNum = 300;
var fireSizeX = 100;
var fireSizeY = 20;
var fires = [];

//propogation parameters
var propogationThreshold = 0.985;
var propogationX = 20;
var propogationY = 40;

//update parameters
var updateInterval = 50;
var timeMultiplier = updateInterval/1000;

var fireTypes = ["small_wildfire", "med_wildfire", "large_wildfire", "wildfire_firestorm"];

//get a random fire entity
function randomFireSize(){
    var level = 0;
    var selector = (getPRNG()*fireTypes.length);

    if( selector > 2.5 && selector < 3.25 ){ level = 1; }
    else if( selector > 3.25 && selector < 3.9 ){ level = 2; }
    else if( selector > 3.9 ){ level = 3; }

    var entityName = fireTypes[level];
    // $console.info_log(entityName + " - " + selector);
    return { name: entityName, intensity: level };
}

//delete all of the fires
function deleteFires(){
    if(fires.length > 0){
        for(var i = 0; i < fires.length; i++){
            var fire = fires[i].entity;
            if(fire != undefined){
                fire.remove();
            }
        }
    }
    deleteFireSketch();
    $console.info_log("Delete Fires");
}

//delete the index in the fires array
function deleteFireIndex(index){
    fires.splice(index, 1);
    //$console.error_log("Remove fire at index " + index);
}

//create a fire at world position, store the entity, guid and intensity of the fire in the fires array
function createFire(posTo, posFrom){
    if(fires.length < fireMaxNum){
        var randomFire = randomFireSize();
        var descriptor = $world.getEntityDescriptorFromName(randomFire.name);//"big_tree_01");
        var fire = $scenario.createEntityECEF(descriptor, posTo);
        var id = fire.getGUID();
        fire.attachToParent($entity);
        fire.snapToGround();
        var fireStruct = { entity: fire, guid: id, intensity: randomFire.intensity }
        fires.push(fireStruct);

        if(sketchFireVisualization && posFrom != null){
          sketchFirePropagation(posFrom, fire.getPositionECEF(), randomFire.intensity);
        }

        //$console.error_log("New " + randomFire.name + " - Total: " + fires.length);
    }
}

//update weather every five seconds
var windParam;
function updateWeather(){
    fireEvents.setTimeout(function() {
        windParam = $scenario.getWindParams($scenario.getTimeOfDay(), $scenario.getDayOfYear(), $entity.getPositionECEF());
        updateWeather();
    }, 5000);
}

//when the parent is moved or rotated snap the children to the ground
function updateFirePos(){
    if(fires.length > 0){
        for(var i = 0; i < fires.length; i++){
            var fire = fires[i].entity;
            if(fire != undefined){
                fire.snapToGround();
            }
        }
    }
    // $console.info_log("Update Fire Pos");
}

function updateFireSystem(){
    //$console.info_log("Wind Speed: " + windParam.intensity + " - Heading: " + windHdgDeg);

    fireEvents.setTimeout(function() {

        var newFirePos = $entity.getPositionECEF();
        if(firePos.x != newFirePos.x && firePos.y != newFirePos.y && firePos.z != newFirePos.z){
            firePos = newFirePos;
            updateFirePos();
        }

        var newFireHdg = $entity.getHeading();
        if(fireHdg != newFireHdg){
            fireHdg = newFireHdg;
            updateFirePos();
        }

        // var fireMovement = { x: getRandomRange(0.005*windParam.intensity*timeMultiplier), y: (getPRNG()*0.1*windParam.intensity*timeMultiplier), z: 0 };
        // fire.translate(fireMovement);
        
        if(fires.length > 0){
            var fireIndex = (getPRNG()*(fires.length-1)).toFixed(0);

            //check to see if the fire entity still exists
            var guid = $scenario.getEntityByGUID(fires[fireIndex].guid);

            //if the entity exists propogate fires
            if(guid != null){
              //$console.error_log("Fire exists");
              var fire = fires[fireIndex].entity;
              var intensity = fires[fireIndex].intensity;
              var propogate = true;
              var propogationSeed = getPRNG();
              var windFactor = windParam.intensity/130;
              var intensityFactor = ((intensity+2)/fireTypes.length);
              var threshold = propogationThreshold-((intensity*0.01)*windFactor);
              //$console.info_log("propogationSeed " + propogationSeed + " - threshold " + threshold + " - intensityFactor " + intensityFactor);

              if(propogationSeed > threshold){ propogate = true; }
              if(propogate){
                  fire.setHeading(rad2deg(windParam.direction));
                  var posX = getRandomRange(propogationX*windParam.intensity*intensityFactor*timeMultiplier);
                  var posY = getPRNG()*propogationY*windParam.intensity*intensityFactor*timeMultiplier; // forward in direction of wind
                  var posECEF = fire.getPositionWorld({x:posX, y:posY, z:0});
                  createFire(posECEF, fire.getPositionECEF());
                  //$console.error_log("X: " + posX.toFixed(2) + " - Y: " + posY.toFixed(2));
              }
            }

            // if the entity does not exist remove the index from the fires array
            else {
              deleteFireIndex(fireIndex);
            }
        }
        updateFireSystem();
    }, updateInterval);
}

function initializeFireSystem(){

    firePos = $entity.getPositionECEF();
    fireHdg = $entity.getHeading();
    windParam = $scenario.getWindParams($scenario.getTimeOfDay(), $scenario.getDayOfYear(), firePos);
    fireEvents = $entity.getEventSystem();
    fireEvents.bindCallbackFunction("Entity_RemovedFromScenario", function () {
      deleteFires();
    });

    for(var i = 0; i < fireNum; i++){
      for (var j = 0; j < fireNum; j++)
      {
        var posX = i*5 - (fireNum/2)*5;
        var posY = j*5 - (fireNum/2)*5;//getRandomRange(fireSizeX);
        //var posY = //getRandomRange(fireSizeY);
        var posECEF = $entity.getPositionWorld({x:posX, y:posY, z:0});
        createFire(posECEF, null);
      }
    }
    updateFireSystem();
}

/**
    Called when scenario starts (or called on placement if scenario already started)
*/
function event_onStart() {
    initializeFireSystem();
    createFireSketch();
    updateWeather();
}