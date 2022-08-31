// Distance between the cell centers. 35mm allow you to easily grab the cells.
period=35;

// Diameter of the battery
batteryDiameter=18;

// number of holder units
units=2;

// add slides for the battery plate/slide 
plateSlide=true;

// add a hole to use a screw to connect to the battery terminals
screw=false;

/* Hidden */
$fn=10;
module mountingUnit(){
    //translate([0,0,0]) rotate([-90,0,0]) cylinder(h=65.0,d=18);
    translate([0,-5,0]) difference() {
        union(){
            translate([0,-25/2,0]) cube([period+2.51,25,20], center=true);
            
            // plateSlide holder
            if (plateSlide) translate([0,0.99,0]) cube([14.5,2,20], center=true);
        }
        
        translate([0,-30.01/2,2]) cube([period-2.5,20,20], center=true);
        
        if (screw){
        // hex
        translate([0,-3,0]) rotate([-90,0,0]) cylinder(h=4, d=6.35, $fn=6);
        
        // screw hole
        translate([0,-6,0]) rotate([-90,0,0]) cylinder(h=7, d=3.5);
        
        // wire hole
        translate([period/2-4,-6,0]) rotate([-90,0,0]) cylinder(h=7, d=3.5);
               
        // wire opening
        translate([period/4-2,0.5,0]) cube([period/2,3,3.5], center=true);
        }
        
        // 45 deg sides
        translate([0,-22,10]) rotate([45,0,0]) cube([period+7,40,20], center=true);
        
        // mounting slot
        translate([0,-18,-15]) rotate([0,0,0]) cylinder(h=20, d=5);
        translate([0,-12,-15]) rotate([0,0,0]) cylinder(h=20, d=5);
        translate([-2.5,-18,-15]) cube([5,6,10]);
        
        if (plateSlide){
            // plateSlide slide
            translate([0,0.49,4.5-10]) cube([10.5,1,20], center=true);
            
            // plateSlide middle
            translate([0,1.49,0]) cube([7.5,3,25], center=true);
        } else {
            // plateSlide position
            if (!screw) translate([0,0,1]) cube([11,1,12], center=true);
        }
        
        // plateSlide lead
        translate([0,-4,14.5]) cube([4,10,20], center=true);
        
        // reduce height
        translate([-period/2-5,-15,5.5]) cube([period+10,20,20]);

    }
}
for (i=[1:units]){
    translate([(period)*(i-1),0,0])    mountingUnit();
}
