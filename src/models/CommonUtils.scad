// Rotation
module rotate_x(x,y,z,angle) {
    translate([x,y,z])
        rotate([angle,0,0])
            translate([-x,-y,-z])
                children();
}
module rotate_y(x,y,z,angle) {
    translate([x,y,z])
        rotate([0,angle,0])
            translate([-x,-y,-z])
                children();
}
module rotate_z(x,y,z,angle) {
    translate([x,y,z])
        rotate([0,0,angle])
            translate([-x,-y,-z])
                children();
}

// Oval

module oval(x1, x2, y1, y2, z1, z2) {
	translate([(x1+x2)/2,(y1+y2)/2,z1]) scale([x2-x1,y2-y1,z2-z1]) cylinder(r=0.5,h=1, $fn=40);
}

// Cone
module cone_z(x,y,z1,z2,rad1,rad2) {
   translate([x,y,z1]) {
		cylinder(r1=rad1, r2=rad2, h = z2-z1);
	}
}
module cylinder_x(x1,x2,y,z,rad1,rad2) {

	rounded_box_x(x1,x2,y-rad,y+rad,z-rad,z+rad,rad);
}

// Cylinders

module cylinder_x(x1,x2,y,z,rad) {
	rounded_box_x(x1,x2,y-rad,y+rad,z-rad,z+rad,rad);
}

module cylinder_y(x,y1,y2,z,rad) {
	rounded_box_y(x-rad,x+rad,y1,y2,z-rad,z+rad,rad);
}

module cylinder_z(x,y,z1,z2,rad) {
	rounded_box(x-rad,x+rad,y-rad,y+rad,z1,z2,rad);
}

// Boxes
module rect(x1, x2, y1, y2, z1, z2) {
	box(x1, x2, y1, y2, z1, z2);
}
	
module box(x1, x2, y1, y2, z1, z2) {
	translate([x1,y1,z1]) cube([x2-x1,y2-y1,z2-z1]);
}
                                                                   
// X Boxes

module rounded_box_x(x1, x2, y1, y2, z1, z2, rad) {
	rotate([0,-90,0]) rounded_box_z(z1, z2, y1, y2, -x2, -x1, rad);
}

module half_rounded_box_xup(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1, y2, z1-rad, z2, rad);
	}
}

module half_rounded_box_xdown(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1, y2, z1, z2+rad, rad);
	}
}
module half_rounded_box_xfront(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1, y2+rad, z1, z2, rad);
	}
}

module half_rounded_box_xback(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1-rad, y2, z1, z2, rad);
	}
}

module quart_rounded_box_xupfront(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1, y2+rad, z1-rad, z2, rad);
	}
}

module quart_rounded_box_xupback(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1-rad, y2, z1-rad, z2, rad);
	}
}

module quart_rounded_box_xdownfront(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1, y2+rad, z1, z2+rad, rad);
	}
}

module quart_rounded_box_xdownback(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_x(x1, x2, y1-rad, y2, z1, z2+rad, rad);
	}
}

// Y Boxes

module rounded_box_y(x1, x2, y1, y2, z1, z2, rad) {
	rotate([90,0,0]) rounded_box_z(x1, x2, z1, z2, -y2, -y1, rad);
}

module half_rounded_box_yup(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1, x2, y1, y2, z1-rad, z2, rad);
	}
}

module half_rounded_box_ydown(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1, x2, y1, y2, z1, z2+rad, rad);
	}
}

module half_rounded_box_yleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1, x2+rad, y1, y2, z1, z2, rad);
	}
}

module half_rounded_box_yright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1-rad, x2, y1, y2, z1, z2, rad);
	}
}

module quart_rounded_box_yupleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1, x2+rad, y1, y2, z1-rad, z2, rad);
	}
}

module quart_rounded_box_yupright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1-rad, x2, y1, y2, z1-rad, z2, rad);
	}
}

module quart_rounded_box_ydownleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1, x2+rad, y1, y2, z1, z2+rad, rad);
	}
}

module quart_rounded_box_ydownright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2, rad);
		rounded_box_y(x1-rad, x2, y1, y2, z1, z2+rad, rad);
	}
}

// Z Boxes

module rounded_box_z(x1, x2, y1, y2, z1, z2, rad) {
	rect(x1, x2, y1+rad, y2-rad, z1, z2);
	rect(x1+rad, x2-rad, y1, y2, z1, z2);

	oval(x1, x1+rad*2, y1, y1+rad*2, z1, z2);
	oval(x1, x1+rad*2, y2-rad*2, y2, z1, z2);
	oval(x2-rad*2, x2, y2-rad*2, y2, z1, z2);
	oval(x2-rad*2, x2, y1, y1+rad*2, z1, z2);
}

module half_rounded_box_zfront(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1, x2, y1, y2+rad, z1, z2, rad);
	}
}

module half_rounded_box_zback(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1, x2, y1-rad, y2, z1, z2, rad);
	}
}

module half_rounded_box_zleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1, x2+rad, y1, y2, z1, z2, rad);
	}
}

module half_rounded_box_zright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1-rad, x2, y1, y2, z1, z2, rad);
	}
}

module quart_rounded_box_zfrontleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1, x2+rad, y1, y2+rad, z1, z2, rad);
	}
}

module quart_rounded_box_zfrontright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1-rad, x2, y1, y2+rad, z1, z2, rad);
	}
}

module quart_rounded_box_zbackleft(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1, x2+rad, y1-rad, y2, z1, z2, rad);
	}
}

module quart_rounded_box_zbackright(x1, x2, y1, y2, z1, z2, rad) {
	intersection() {
		box(x1, x2, y1, y2, z1, z2);
		rounded_box_z(x1-rad, x2, y1-rad, y2, z1, z2, rad);
	}
}

// Legacy

module rounded_box(x1, x2, y1, y2, z1, z2, rad) {
	rounded_box_z(x1, x2, y1, y2, z1, z2, rad);
}

module half_rounded_box(x1, x2, y1, y2, z1, z2, rad) {
	half_rounded_box_zfront(x1, x2, y1, y2, z1, z2, rad);
}

module half_rounded_box_x(x1, x2, y1, y2, z1, z2, rad) {
	half_rounded_box_xfront(x1, x2, y1, y2, z1, z2, rad);
}

module half_rounded_box_y(x1, x2, y1, y2, z1, z2, rad) {
	half_rounded_box_yup(x1, x2, y1, y2, z1, z2, rad);
}



// Frustum

// 1(5)  2(6)
// 0(4)  3(7)
module frustrum(dx1,dy1,dx2,dy2,dz1,dz2) {
	frustum(dx1,dy1,dx2,dy2,dz1,dz2);
}
	
module frustum(dx1,dy1,dx2,dy2,dz1,dz2) {
	polyhedron( points=[[-dx1/2,-dy1/2,dz1],
						 [-dx1/2, dy1/2,dz1],
						 [ dx1/2, dy1/2,dz1],
						 [ dx1/2,-dy1/2,dz1],
						 [-dx2/2,-dy2/2,dz2],
						 [-dx2/2, dy2/2,dz2],
						 [ dx2/2, dy2/2,dz2],
						 [ dx2/2,-dy2/2,dz2]],
				 triangles=[[0,2,1],[0,3,2],
							 [0,4,7],[0,7,3],
							 [3,7,6],[3,6,2],
							 [2,6,5],[2,5,1],
							 [1,5,4],[1,4,0],
							 [4,5,6],[4,6,7]] );
}
