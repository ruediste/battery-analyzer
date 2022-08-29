$fn=10;
period=35;
units=1;

module mountingUnit(){
    *translate([0,0,0]) rotate([-90,0,0]) cylinder(h=65.0,d=18);
    translate([0,-5,0]) difference() {
        union(){
            translate([0,-25/2,0]) cube([period+2.51,25,20], center=true);
            // clip holder
            translate([0,0.99,0]) cube([14.5,2,20], center=true);
        }
        
        translate([0,-30.01/2,2]) cube([period-2.5,20,20], center=true);
        
        // hex
        translate([0,-3,0]) rotate([-90,0,0]) cylinder(h=4, d=6.35, $fn=6);
        
        // screw hole
        translate([0,-6,0]) rotate([-90,0,0]) cylinder(h=7, d=3.5);
        
        // wire hole
        translate([period/2-4,-6,0]) rotate([-90,0,0]) cylinder(h=7, d=3.5);
        
        // 45 deg sides
        translate([0,-22,10]) rotate([45,0,0]) cube([period+7,40,20], center=true);
        
        // mounting hole
        translate([0,-15,-15]) rotate([0,0,0]) cylinder(h=20, d=5);
        
        // clip slide
        translate([0,0.49,0]) cube([10.5,1,25], center=true);
        
        // clip middle
        translate([0,1.49,0]) cube([7.5,3,25], center=true);
    }
}
for (i=[1:units]){
    translate([(period)*(i-1),0,0])    mountingUnit();
}