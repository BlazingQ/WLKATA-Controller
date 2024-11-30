#include<iostream>
#include<string.h>
#include<vector>

using namespace std;

std::vector<std::string> msgs1 = {
    "M20 G90 G00 X198.6 Y0 Z230.7 A0 B-90 C0",
    "M20 G90 G00 X0 Y169.1 Z230.6 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y190.3 Z232.5 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y260 Z233 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y260 Z257.5 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y160.5 Z257.5 A0 B-109.9 C90",
    "M20 G90 G00 X0 Y-160.8 Z257.5 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-245 Z83.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-245 Z65.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-160 Z65.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-160 Z120 A0 B-90 C-90",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000",
    "M20 G90 G00 X198.8 Y0 Z230.7 A0 B-90 C0"
};

vector<string> msgs2 = {
    "M20 G90 G00 X198.67 Y0 Z230.72 A0 B0 C0",
    "M20 G90 G00 X163.5 Y1.9 Z71.8 A0 B0 C0",
    "M20 G90 G00 X165 Y1.9 Z56.8 A0 B0 C0",
    "M20 G90 G00 X168.67 Y0 Z126.72 A0 B0 C0",
    "M20 G90 G00 X34.1 Y-226.9 Z146.1 A0 B0 C90",
    "M20 G90 G00 X10.5 Y-232.9 Z134.8 A0 B3.9 C90",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000"
};

vector<string> msgs3 = {
    "M20 G90 G00 X0 Y-160 Z185 A0 B-25 C-90",
    "M20 G90 G00 X-5 Y-195 Z170 A0 B-25 C-90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M20 G90 G00 X10 Y-195 Z170 A0 B-25 C-90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M20 G90 G00 X25 Y-195 Z170 A0 B-25 C-90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M20 G90 G00 X183.67 Y0 Z230 A0 B0 C0"
};

vector<string> msgs4 = {
    "M20 G90 G00 X0 Y160 Z185 A0 B-25 C90",
    "M20 G90 G00 X-10 Y198 Z165 A0 B-25 C90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M20 G90 G00 X5 Y198 Z165 A0 B-25 C90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M20 G90 G00 X20 Y198 Z165 A0 B-25 C90",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000"
};


// arm5：
vector<string> msgs5 = {
    "M20 G90 G00 X-15 Y173 Z150.4 A0 B0.1 C90",
    "M20 G90 G00 X-10 Y173 Z106.1 A0 B0 C90",
    "M20 G90 G00 X-15 Y173 Z115.4 A0 B0.1 C90",
    "M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90",
    "M20 G90 G00 X10 Y173 Z108.8 A0 B0 C90",
    "M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90",
    "M20 G90 G00 X25.1 Y173 Z114.8 A0 B0.1 C90",
    "M20 G90 G00 X29.9 Y172.9 Z109.5 A0 B0.1 C90",
    "M20 G90 G00 X25.1 Y178.6 Z114.8 A0 B0.1 C90",
    "M20 G90 G00 X25 Y283.6 Z129.5 A0 B0 C270",
    "M20 G90 G00 X25 Y283.5 Z116.2 A0 B0 C-90",
    "M20 G90 G00 X25 Y283.4 Z115.9 A0 B0 C-90",
    "M20 G90 G00 X5 Y283.6 Z114.2 A0 B0 C-90",
    "M20 G90 G00 X9.9 Y283.5 Z116.2 A0 B0 C-90",
    "M20 G90 G00 X5 Y283.6 Z115.9 A0 B0 C-90",
    "M20 G90 G00 X-15 Y283.6 Z113.9 A0 B0 C-80",
    "M20 G90 G00 X-9.9 Y283.5 Z112.2 A0 B0 C-79.9",
    "M20 G90 G00 X198.67 Y0 Z230.72 A0 B0 C0"
};

std::vector<std::string> msgs1_c = {
    "M20 G90 G00 X198.6 Y0 Z230.7 A0 B-90 C0",
    "M20 G90 G00 X0 Y169.1 Z230.6 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y190.3 Z232.5 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y260 Z233 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y260 Z257.5 A0 B-89.9 C90",
    "M20 G90 G00 X0 Y160.5 Z257.5 A0 B-109.9 C90",
    "M20 G90 G00 X0 Y-160.8 Z257.5 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-245 Z83.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-245 Z65.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-160 Z65.4 A0 B-90 C-90",
    "M20 G90 G00 X0 Y-160 Z120 A0 B-90 C-90",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000",
    "M20 G90 G00 X198.8 Y0 Z230.7 A0 B-90 C0"
};

vector<string> msgs2_c = {
    "M20 G90 G00 X198.67 Y0 Z230.72 A0 B0 C0",
    "M20 G90 G00 X163.5 Y1.9 Z71.8 A0 B0 C0",
    "M3S1000",
    "M20 G90 G00 X165 Y1.9 Z56.8 A0 B0 C0",
    "M20 G90 G00 X168.67 Y0 Z126.72 A0 B0 C0",
    "M20 G90 G00 X34.1 Y-226.9 Z146.1 A0 B0 C90",
    "M20 G90 G00 X10.5 Y-232.9 Z134.8 A0 B3.9 C90",
    "M3S0",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000"
};

vector<string> msgs3_c = {
    "M20 G90 G00 X0 Y-160 Z185 A0 B-25 C-90",
    "M20 G90 G00 X-5 Y-195 Z170 A0 B-25 C-90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M20 G90 G00 X10 Y-195 Z170 A0 B-25 C-90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M20 G90 G00 X25 Y-195 Z170 A0 B-25 C-90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M20 G90 G00 X183.67 Y0 Z230 A0 B0 C0"
};

vector<string> msgs4_c = {
    "M20 G90 G00 X0 Y160 Z185 A0 B-25 C90",
    "M20 G90 G00 X-10 Y198 Z165 A0 B-25 C90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M20 G90 G00 X5 Y198 Z165 A0 B-25 C90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M20 G90 G00 X20 Y198 Z165 A0 B-25 C90",
    "M3S1000",
    "F200",
    "M20 G91 G01 X0 Y0 Z-20 A0 B0 C0",
    "M3S0",
    "M20 G91 G01 X0 Y0 Z20 A0 B0 C0",
    "F2000",
    "M21 G90 X0 Y0 Z0 A0 B0 C0 F2000"
};


// arm5：
vector<string> msgs5_c = {
    "M20 G90 G00 X-15 Y173 Z150.4 A0 B0.1 C90",
    "M20 G90 G00 X-10 Y173 Z106.1 A0 B0 C90",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X-15 Y173 Z115.4 A0 B0.1 C90",
    "M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90",
    "M20 G90 G00 X10 Y173 Z108.8 A0 B0 C90",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90",
    "M20 G90 G00 X25.1 Y173 Z114.8 A0 B0.1 C90",
    "M20 G90 G00 X29.9 Y172.9 Z109.5 A0 B0.1 C90",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X25.1 Y178.6 Z114.8 A0 B0.1 C90",
    "M20 G90 G00 X25 Y283.6 Z129.5 A0 B0 C270",
    "M20 G90 G00 X25 Y283.5 Z116.2 A0 B0 C-90",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X25 Y283.4 Z115.9 A0 B0 C-90",
    "M20 G90 G00 X5 Y283.6 Z114.2 A0 B0 C-90",
    "M20 G90 G00 X9.9 Y283.5 Z116.2 A0 B0 C-90",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X5 Y283.6 Z115.9 A0 B0 C-90",
    "M20 G90 G00 X-15 Y283.6 Z113.9 A0 B0 C-80",
    "M20 G90 G00 X-9.9 Y283.5 Z112.2 A0 B0 C-79.9",
    "M3S1000",
    "M3S0",
    "M20 G90 G00 X198.67 Y0 Z230.72 A0 B0 C0"
};

string joinMessages(vector<string> msgs) {
    std::string result;
    for(int i = 0; i < msgs.size(); ++i) {
        result += msgs[i];
        if (i < msgs.size() - 1) {
            result += ",";
        }
    }
    return result;
}

int main(){
    cout<<"msg1 = "<<joinMessages(msgs1)<<endl;
    cout<<msgs1.size()<<endl;
    cout<<"msg2 = "<<joinMessages(msgs2)<<endl;
    cout<<msgs2.size()<<endl;
    cout<<"msg3 = "<<joinMessages(msgs3)<<endl;
    cout<<msgs3.size()<<endl;
    cout<<"msg4 = "<<joinMessages(msgs4)<<endl;
    cout<<msgs4.size()<<endl;
    cout<<"msg5 = "<<joinMessages(msgs5)<<endl;
    cout<<msgs5.size()<<endl;
    return 0;
}



