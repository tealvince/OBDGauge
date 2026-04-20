include <commonutils.scad>

button_spacing = 6.5;
button_diam = 4+1;
button_y = -7.5-4.89+1;

ring_retainer_lip_diam = 34;
ring_retainer_lip_thick = 3;
ring_retainer_inner_diam = 30-1.2+2.3;
ring_retainer_height = 10.8-2;
ring_retainer_notch_height = 31.5;
ring_retainer_notch_width = 0.5;

ring_con_x = 12.6;
ring_con_y = 22.5/2;
ring_con_w = 10;
ring_con_h = 3.5;

led_cy = 1;
led_width = 31+.5;
led_height = 15;//-2.7;
led_lip_v_width = 1;
led_lip_h_width = 2;

light_dx = -9;
light_dy = -8.5;
light_hole_diam = 2;
light_rx = 15;
light_ry = -15;

shell_in_lip = 2.5;
shell_out_lip = 3;
shell_diam = 50+0.5;
shell_height = 40+15-4.5;
shell_thick = 2;
shell_glass_thick = 3;
shell_front_thick = 6;
shell_holes_dz = 8;
shell_holes_diam = 4;

usb_depth = 8;
usb_width = 12;
usb_z = 47-3;

// Angled back
back_angle = 45;
back_angle_x = shell_diam/2;
back_angle_z = usb_z+usb_depth/2+3;

obd_width = 9.3;
obd_depth = 5;
obd_angle = 180-45-90+17;
obd_z = shell_height + shell_glass_thick + shell_front_thick + shell_thick - obd_depth - 1;

//button();
// ring_retainer();

color("blue") shell();

//rotate([180])
//  translate([0,0,20])
//      color("red") back();

//led_support();
//  grille();

module led_support() {    
    diam = 49;
    height = 7.5;
    thick = 2;
    depth = 10;
    
    board_width = 22.15+.25+.25;
    board_thick = 1.6;

    difference() {
        intersection() {
            cylinder_z(0,0,0,height+board_thick+thick+5, diam/2);
            box(-board_width/2-thick, board_width/2+thick, -100, 100, -100, 100);
        }
        intersection() {
            cylinder_z(0,0,0,height+board_thick+thick+5, diam/2-thick);
            box(-board_width/2+1, board_width/2-1, -100, 100, -100, 100);
        }
        box(-board_width/2, board_width/2, -100, 100, height, height+board_thick);
        box(-100,100, -diam/2+depth, diam/2-depth, -100,100);
        box(-100,-board_width/2, 0, diam/2-5, -100,100);
    }
}

module grille() {
    inner_diam = 34;
    outer_diam = 49.5;
    thick = 2.8;
    slit = 2.5-1;
    
    translate([0,0,-ring_retainer_lip_thick])
    difference() {
        cylinder_z(0,0,0,thick/2,outer_diam/2);
        cylinder_z(0,0,thick/2,thick,outer_diam/2-2);
        cylinder_z(0,0,0,thick,inner_diam/2);
        for (angle = [0: 360/16: 360]) rotate([0,0,angle]) {
            box(-slit/2,slit/2, inner_diam/2+1, outer_diam/2-2, 0,thick+2);
        }
    }
}

module back() {
    
    // center post support
    cylinder_z(-4, 0,
        shell_height + shell_glass_thick + shell_front_thick - 13, 
        shell_height + shell_glass_thick + shell_front_thick + shell_thick,
                4);

    // Angled back
    difference() {
        union() {
            // outer
            difference() {
                intersection() {
                    translate([back_angle_x,0,back_angle_z])
                        rotate([0, back_angle, 0])
                            box(-100,100,-100,100,0,100);
                    cylinder_z(0,0,
                        0, 
                        shell_glass_thick + shell_front_thick+shell_height+shell_thick,
                        shell_diam/2+shell_thick);
                }

                translate([back_angle_x,0,back_angle_z])
                    rotate([0, back_angle, 0])
                        box(-100,100,-100,100,shell_thick,100);
            }       
            // inner 
            intersection() {
                difference() {
                    cylinder_z(0,0,
                        0, 
                        shell_glass_thick + shell_front_thick+shell_height+shell_thick,
                        shell_diam/2-.25);
                    cylinder_z(0,0,
                        0, 
                        shell_glass_thick + shell_front_thick+shell_height+shell_thick,
                        shell_diam/2-shell_thick);
                }
                translate([back_angle_x,0,back_angle_z])
                    rotate([0, back_angle, 0])
                        box(-100,100,-100,100,-shell_thick,.01);
            }        
        }
        rounded_box_z(shell_diam/2-usb_depth, shell_diam/2,
            -usb_width/2, usb_width/2, -100,100, usb_depth/2);
    }
    
    difference() {
        union() {
            // Inner
            cylinder_z(0,0,
                shell_height+ shell_glass_thick + shell_front_thick-shell_holes_dz-5, 
                shell_glass_thick + shell_front_thick+shell_height+shell_thick,
                shell_diam/2-.25);
            
            // lid
            cylinder_z(0,0,
                shell_glass_thick + shell_front_thick+shell_height,
                shell_glass_thick + shell_front_thick+shell_height+shell_thick,
                shell_diam/2+shell_thick);

        }
        // Angled Back
        translate([back_angle_x,0,back_angle_z])
            rotate([0, back_angle, 0])
                box(-100,100,-100,100,shell_thick,100);

        // tab cutouts
        box(-100,-8, -100,100, 0, shell_height+ shell_glass_thick + shell_front_thick-shell_thick);
        box(8, 100, -100,100, 0, shell_height+ shell_glass_thick + shell_front_thick- shell_thick);
        box(-100, 100, -shell_diam/2+2.5+2, shell_diam/2-2.5-2, 0, shell_height+ shell_glass_thick + shell_front_thick- shell_thick);

        // inner circle
            cylinder_z(0,0,0,
                    shell_glass_thick + shell_front_thick+shell_height,
                    shell_diam/2-shell_thick);
        
        // Screw holes
        translate([0, -shell_diam/2-shell_thick+2, 
            shell_height+ shell_glass_thick + shell_front_thick-shell_holes_dz]) 
                rotate([-90]) shell_hole();
        mirror([0,1,0]) 
            translate([0, -shell_diam/2-shell_thick+2, 
            shell_height+ shell_glass_thick + shell_front_thick-shell_holes_dz]) 
                rotate([-90]) shell_hole();
    }
}    

module shell() {
    difference(){
        union() {
            cylinder_z(0,0,0, 
                shell_height + shell_glass_thick + shell_front_thick,
                shell_diam/2+shell_thick);
            cylinder(
                r1 = shell_diam/2+shell_thick, 
                r2 = shell_diam/2+shell_thick+shell_out_lip,
                h = shell_front_thick/2);
            cylinder_z(0,0,shell_front_thick/2-.01, 
                shell_front_thick,
                shell_diam/2+shell_thick+shell_out_lip);
        }
        cylinder_z(0,0,
            shell_front_thick-.01, 
            shell_glass_thick + shell_front_thick+shell_height+.01,
            shell_diam/2);
        cylinder_z(0,0,-0.01,
            shell_front_thick,
            shell_diam/2-shell_in_lip);

        // usb port
        rounded_box_x(0, 100, -usb_width/2, usb_width/2,
            usb_z-usb_depth/2, usb_z+usb_depth/2, usb_depth/2);
        
        // obd port
        rotate([0,0,obd_angle]) union() {
            rounded_box_x(0, 100, -obd_width/2, obd_width/2,
                obd_z-obd_depth/2, obd_z+obd_depth/2, obd_depth/2);
            box(0, 100, -2, 2,
                obd_z-obd_depth/2, obd_z+obd_depth/2+5);
        }
        
        // Screw holes
        translate([0, -shell_diam/2-shell_thick, 
            shell_height+ shell_glass_thick + shell_front_thick-shell_holes_dz]) 
                rotate([-90]) shell_hole();
        mirror([0,1,0]) 
            translate([0, -shell_diam/2-shell_thick, 
            shell_height+ shell_glass_thick + shell_front_thick-shell_holes_dz]) 
                rotate([-90]) shell_hole();
        
        // Angled back
        
        translate([back_angle_x,0,back_angle_z])
            rotate([0, back_angle, 0])
                box(-100,100,-100,100,0,100);
    }
} 

module shell_hole() {
        cylinder_z(0,0,0,
            shell_thick*2+1,
            shell_holes_diam/2);
        cylinder(
            r1 = shell_holes_diam/2+shell_thick/2, 
            r2 = shell_holes_diam/2,
            h = shell_thick/2);

}

module button() {
    cylinder_z(0,0,0,5, 4.5/2);
    cylinder_z(0,0,0,12, 3/2);

}

module ring_retainer() {
    difference() {
        union() {
            cylinder_z(0, 0, -ring_retainer_lip_thick, 0, ring_retainer_lip_diam/2);
            cylinder_z(0, 0, 0, ring_retainer_height, ring_retainer_inner_diam/2);
            box(-ring_retainer_notch_width/2, ring_retainer_notch_width/2, 
                -ring_retainer_notch_height/2, ring_retainer_notch_height/2, 
                0, ring_retainer_height);
        }
        
        // LED
        box(-led_width/2, led_width/2, led_cy-led_height/2, led_cy+led_height/2, 0, 100);
        box(-led_width/2+led_lip_v_width, led_width/2-led_lip_v_width, 
            led_cy-led_height/2+led_lip_h_width, 
            led_cy+led_height/2-led_lip_h_width, 
            -ring_retainer_lip_thick, 100);
        box(-led_width/2, led_width/2-led_lip_v_width, 
            led_cy-led_height/2+led_lip_h_width, 
            led_cy-led_height/2+led_lip_h_width+2, 
            -ring_retainer_lip_thick, 100);
        
        // Button holes
        cylinder_z(-button_spacing/2, button_y, -10,100, button_diam/2);
        cylinder_z( button_spacing/2, button_y, -10,100, button_diam/2);
        
        // Connector slot
        box(ring_con_x, ring_con_x + ring_con_w,
            ring_con_y, ring_con_y + ring_con_h, 0, 100);
        
        // light_hole
        translate([light_dx, light_dy])
            rotate([light_rx, light_ry])
                cylinder_z(0,0, -100, 100, light_hole_diam/2);
                
        // mirrored for appearance
        mirror([1,0,0]) translate([light_dx, light_dy])
            rotate([light_rx, light_ry])
                cylinder_z(0,0, -100, 100, light_hole_diam/2);
    }
}