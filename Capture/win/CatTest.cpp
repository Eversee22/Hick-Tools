#include"Cat.h"
int main() {
	HickCat hc;
	int f;
	cout << "choose filter:\n"
		"1.TCP|2.UDP|3.ICMP|4.TCP and UDP|5.TCP and ICMP|6.UDP and ICMP|0.ALL\n";
	cin >> f;
	switch (f) {
	case 0:
		hc.setAll(true);
		break;
	case 1:
		hc.setTCP(true); 
		break;
	case 2:
		hc.setUDP(true); 
		break;
	case 3:
		hc.setICMP(true); 
		break;
	case 4:
		hc.setTCP(true);
		hc.setUDP(true);
		break;
	case 5:
		hc.setTCP(true);
		hc.setICMP(true);
		break;
	case 6:
		hc.setUDP(true);
		hc.setICMP(true);
		break;
	default:
		cout << "catch nothing";
	}
	hc.startCat();

	return 0;

}