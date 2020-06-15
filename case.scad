base();
%top();


module base(){

difference() {
cube([65,45,30], center = true);
translate([0, 0, 5]) cube([61,41,30], center = true);

translate([0, 0, 5]) cube([5,20,30], center = true);
 
translate([-20, 0, -14]) cube([3,35,10], center = true);   
translate([20, 0, -14]) cube([3,35,10], center = true);
translate([10, 0, -14]) cube([3,35,10], center = true);
translate([-10, 0, -14]) cube([3,35,10], center = true);
translate([0, 0, -14]) cube([3,35,10], center = true);
  
translate([10, 22,20]) cube([30,5,30], center = true); 
 translate([31.5, -15,20]) cube([2.5,2.5,30], center = true);  
  

}

translate([29, 19, -2.5]) cube([3,3,15], center = true);
translate([-29, 19, -2.5]) cube([3,3,15], center = true);
translate([-29, -19, -2.5]) cube([3,3,15], center = true);
translate([29, -19, -2.5]) cube([3,3,15], center = true); 
}







module top(){
difference(){
translate([0, 0, 5]) cube([61,41,5], center = true);
translate([45-25, -4, 5]) cube([21,33,6], center = true);
   
}

translate([28, 18, 1.4]) cube([5,5,3], center = true);
translate([-28, 18, 1.4]) cube([5,5,3], center = true);
translate([-28, -18, 1.4]) cube([5,5,3], center = true);
translate([7, -18, 1.4]) cube([5,5,3], center = true);
}
