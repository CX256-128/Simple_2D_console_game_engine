#pragma once
#include <iostream>
#include<fstream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include<algorithm>
#include<unordered_map>
// 使用非阻塞的函数来获取用户输入的字符
char getcha() {
    if (_kbhit()) {
        return _getch();
    }
    return 0;
}
// 使用Windows API隐藏控制台光标
void HideCursor() {
    CONSOLE_CURSOR_INFO cursor_info = {1, 0}; 
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
// 使用windows API来自由控制控制台上面的光标位置以辅助实现渲染功能
void gotoxy(int x, int y) {
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
struct variable_table{
    const double version = 1.0;
    // general registers
    double xmm0=0.0,xmm1=0.0;
    int eax=0,ebx=0;
    long long rax=0,rbx=0; 
    // time_register is the write_only
    std::vector<int> time_reg;
    // the default set for the player
    int blood=0,power=0,shield=0;
    int max_blood=0,max_power=0,max_shield=0;
    // the current position for the player
    int x=1,y=1;
    int direction=0; // 0: up, 1: right, 2: down, 3: left 
    int sight=3;
    // cmp_flag
    int cmp_flag=2; // 0: equal, 1: greater, -1: less , others means this symbol is not set for further use
    // message
    std::vector<std::string> msg;

    // The symbol for generate some sort of stuff also write_only
    std::vector<int> gen_x,gen_y;
    std::vector<char> gen_sym;

};
inline void read_variable_table(const std::string& filename,variable_table& target){
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr<<"Error opening file: "<<filename<<std::endl;
        return;
    }
    double version;
    file>>version;
    if(version!=target.version){
        std::cerr<<"VERSION MISMATCH"<<std::endl;
        return;
    }
    file>>target.blood>>target.power>>target.shield;
    file>>target.max_blood>>target.max_power>>target.max_shield;
    file>>target.x>>target.y>>target.direction>>target.sight;
    file.close();
    return ;
}
class display_table{
    private:
        const double version = 1.0;
        std::vector<std::vector<char>> display_table;
        int last_x,last_y;
    public:
        std::vector<char> barricade;
        char hidden = ' ';
        char player = '&';
        inline void read(const std::string& filename){
            std::ifstream file(filename);
            // Implementation for reading the file
            if(!file.is_open()){
                std::cerr<<"Error opening file: "<<filename<<std::endl;
                return;
            }
            double version;
            file>>version;
            if(version!=this->version){
                std::cerr<<"VERSION MISMATCH"<<std::endl;
                return;
            }
            int x,y;
            file >> x >> y;
            this->initialize_size(x,y);
            file >> this->player;
            std::string buffer;
            std::getline(file,buffer); // consume the newline after player
            for(int i=0;i<buffer.size();i++){
                if(buffer[i]!=' '){
                    this->barricade.push_back(buffer[i]);
                }
            }
            for(int i=0;i<y;i++){
                if(file.eof()){
                    goto out;
                }
                std::string line;
                std::getline(file,line);
                for(int j=0;j<std::min((int)line.size(),x);j++){
                    this->display_table[i][j]=line[j];
                }
            }
            out:
            file.close();
            return ;
        }
        inline void initialize_size(int col,int row){
            display_table.resize(row);
            for(int i=0;i<row;i++){
                display_table[i].resize(col);
            }
        }
        inline void get_size(int& col,int& row){
            row = this->display_table.size();
            col = this->display_table[0].size();
        }
        char dir[4] = {'^','>','v','<'};
        inline void display(variable_table& target){
            // calculate the sight range
            int x_min = std::max(0,target.x-target.sight);
            int y_min = std::max(0,target.y-target.sight);
            int x_max = std::min((int)display_table[0].size()-1,target.x+target.sight);
            int y_max = std::min((int)display_table.size()-1,target.y+target.sight);
            bool space_padding_x = (x_max-x_min+1)<=this->last_x;
            bool space_padding_y = (y_max-y_min+3)<=this->last_y;
            for(int i=y_min;i<=y_max;i++){
                for(int j=x_min;j<=x_max;j++){
                    std::cout<<this->display_table[i][j];
                }
                if(space_padding_x){
                    for(int k=0;k<40-last_x;k++){
                        std::cout<<' ';
                    }
                }
                std::cout<<'\n';
            }
            std::cout<<"Blood: "<<target.blood<<"  Power: "<<target.power<<"  Shield: "<<target.shield;
            if(space_padding_x){
                for(int k=0;k<35-last_x;k++){
                    std::cout<<' ';
                }
            }
            std::cout<<'\n';
            std::cout<<"X: "<<target.x<<"  Y: "<<target.y<<"  Direction: "<<this->dir[target.direction]<<"  Sight: "<<target.sight;
            if(space_padding_x){
                for(int k=0;k<35-last_x;k++){
                    std::cout<<' ';
                }
            }
            std::cout<<'\n';
            if(space_padding_y){
                for(int k=0;k<this->last_y-(y_max-y_min);k++){
                    for(int i=0;i<35;i++){
                        std::cout<<' ';
                    }
                    std::cout<<'\n';
                }
            }
            this->last_x = x_max-x_min+1;
            this->last_y = y_max-y_min+3;
            gotoxy(0,0);
        }
        inline bool teleport(int x,int y,variable_table& target){
            if(x > target.x){
                target.direction = 1;
            }
            else if(x < target.x){
                target.direction = 3;
            }
            else if(y > target.y){
                target.direction = 2;
            }
            else if(y < target.y){
                target.direction = 0;
            }
            if(std::find(this->barricade.begin(),this->barricade.end(),this->display_table[y][x])!=this->barricade.end()){
                return false;
            }
            if(x < 0 || x >= this->display_table[0].size() || y < 0 || y >= this->display_table.size()){
                return false;
            }
            
            
            // if(this->hidden == ''){
                // this->display_table[target.y][target.x]=' ';
                // target.x = x;
                // target.y = y;
                // this->hidden = this->display_table[y][x];
                // this->display_table[y][x]=this->player;
                // return true;
            // }
            this->display_table[target.y][target.x]=this->hidden;
            target.x = x;
            target.y = y;
            this->hidden = this->display_table[y][x];
            this->display_table[y][x]=this->player;
            return true;
        }
        inline bool initial_teleport(int x,int y,variable_table& target){
            this->hidden = this->display_table[y][x];
            return this->teleport(x,y,target);
        }
        //1. If you want to generate the entity, you can use this function to generate the entity on the map, and you can also set the symbol for the entity
        //   But you have to correlate the entity with the event_tree where its behavior is defined, and you can also set the symbol for the entity
        //2. Or if you want to change the static setting in the display map,you can also use this function
        inline void generate(int x,int y,char symbol){
            this->display_table[y][x]=symbol;
        }
        inline void remove(int x,int y){
            this->display_table[y][x]=' ';
        }
        inline void remove(int x,int y,char symbol){
            this->display_table[y][x]=symbol;
        }
        inline bool generate(const variable_table& target){
            if(target.gen_x.size()!=target.gen_y.size() || target.gen_x.size()!=target.gen_sym.size()){
                std::cerr<<"GENERATION SIZE MISMATCH"<<std::endl;
                return false;
            }
            for(int i=0;i<target.gen_x.size();i++){
                this->display_table[target.gen_y[i]][target.gen_x[i]]=target.gen_sym[i];
            }
            return true;
        }
};
inline void mov(variable_table& target,int value,const std::string& dest){
    if(dest == "power"){
        target.power = value;
    }
    else if(dest == "blood"){
        target.blood = value;
    }
    else if(dest == "shield"){
        target.shield = value;
    }
    else if(dest == "max_power"){
        target.max_power = value;
    }
    else if(dest == "max_blood"){
        target.max_blood = value;
    }
    else if(dest == "max_shield"){
        target.max_shield = value;
    }
    else if(dest == "x"){
        target.x = value;
    }
    else if(dest == "y"){
        target.y = value;
    }
    else if(dest == "gen_x"){
        target.gen_x.push_back(value);
    }
    else if(dest == "gen_y"){
        target.gen_y.push_back(value);
    }
    else if(dest == "time_reg"){
        target.time_reg.push_back(value);
    }
    else if(dest == "direction"){
        target.direction = value;
    }
    else if(dest == "sight"){
        target.sight = value;
    }
    else if(dest == "eax"){
        target.eax = value;
    }
    else if(dest == "ebx"){
        target.ebx = value;
    }
    else{
        std::cerr<<"INVALID DESTINATION"<<std::endl;
        return;
    }
}
inline void mov(variable_table& target,char value,const std::string& dest){
    if(dest == "gen_sym"){
        target.gen_sym.push_back(value);
    }
    else{
        std::cerr<<"INVALID DESTINATION"<<std::endl;
        return;
    }
}
inline void mov(variable_table& target,double value,const std::string& dest){
    if(dest == "xmm0"){
        target.xmm0 = value;
    }
    else if(dest == "xmm1"){
        target.xmm1 = value;
    }
    else{
        std::cerr<<"INVALID DESTINATION"<<std::endl;
        return;
    }
}
inline void mov(variable_table& target,long long value,const std::string& dest){
    if(dest == "rax"){
        target.rax = value;
    }
    else if(dest == "rbx"){
        target.rbx = value;
    }
    else{
        std::cerr<<"INVALID DESTINATION"<<std::endl;
        return;
    }
}
inline double get_number(variable_table& target,const std::string& src){
    if(src == "power"){
        return target.power;
    }
    else if(src == "blood"){
        return target.blood;
    }
    else if(src == "shield"){
        return target.shield;
    }
    else if(src == "max_power"){
        return target.max_power;
    }
    else if(src == "max_blood"){
        return target.max_blood;
    }
    else if(src == "max_shield"){
        return target.max_shield;
    }
    else if(src == "direction"){
        return target.direction;
    }
    else if(src == "sight"){
        return target.sight;
    }
    else if(src == "x"){
        return target.x;
    }
    else if(src == "y"){
        return target.y;
    }
    else if(src == "direction"){
        return target.direction;
    }
    else if(src == "sight"){
        return target.sight;
    }
    else if(src == "eax"){
        return target.eax;
    }
    else if(src == "ebx"){
        return target.ebx;
    }
    if(src == "xmm0"){
        return target.xmm0;
    }
    else if(src == "xmm1"){
        return target.xmm1;
    }
    else{
        std::cerr<<"INVALID SOURCE"<<std::endl;
        return -1.0;
    }
}
inline long long get_long(variable_table& target,const std::string& src){
    if(src == "rax"){
        return target.rax;
    }
    else if(src == "rbx"){
        return target.rbx;
    }
    else{
        std::cerr<<"INVALID SOURCE"<<std::endl;
        return -1;
    }
}

/*struct event_branch{
    std::streampos offset;
    std::string name;
};
Tips: 
 -  The upper structure will be replaced by the unordered_map in the event_tree to achieve the O(1) time complexity for the label locating & searching

*/
class event_tree{
    private:
        std::unordered_map<std::string,std::streampos> branches;
        std::unordered_map<int,std::string> namemap;
        std::ifstream file;
        //std::vector<std::streampos> temp_offsets; // To manage the offsets as the stack to achieve the temporary call and return
    public:
        std::vector<std::vector<int>> trigger_table;
        std::vector<std::ifstream> wait;
        // to open the file for reading
        inline bool open(const std::string& filename){
            file.open(filename, std::ios::in | std::ios::binary);
            if(!file.is_open()){
                return false;
            }
            return true;
        }
        inline void close(){
            file.close();
        }

        // to generate the trigger table: initialize_size() -> build()
        // ANd make sure you have already made the file* open
        inline void initialize_size(int col,int row){
            trigger_table.resize(row);
            for(int i=0;i<row;i++){
                trigger_table[i].resize(col);
            }
        }
        inline void build(){
            if(!file.is_open()){
                return;
            }
            int num;
            file>>num;
            for(int i=1;i<=num;i++){
                int x,y;
                std::string z;
                file>>z;
                file>>x>>y;
                this->trigger_table[y][x]=i;
                this->namemap[i] = z;
            }
            std::string line;
            while (std::getline(this->file, line)) {
                // 若当前行是标签定义，记录该标签的起始偏移
                /*if (!line.empty() && line[0] == ':' ) {
                    
                    auto it = this->branches.find(line.substr(2));
                    if(it != this->branches.end()){   
                        it->second = file.tellg();  
                    }
                    else if(it == this->branches.end()){
                        std::cerr<<"UNDEFINED LABEL"<<std::endl;
                    }
                }*/
                // 注意，标签行不能写注释，所有的注释只能够加在带确定长度限制的指令后方，并使用至少一个空格隔开
                if (!line.empty() && line[0] == ':' ) {
                    if(line.back() == '\r' || line.back() == '\n'){
                        line.pop_back();
                    }
                    std::string label = line.substr(2);
                    this->branches[label] = file.tellg();    // 注意：tellg 在 getline 后指向下一行开头也即正式代码的开头
                }
            }
            this->file.clear(); 
            this->file.seekg(0,std::ios::beg);
        }

        // Now there is the interpreter
        // But first of all we should carefully design the language for the event tree
        // Here we aspire to design a simple language for the event tree
        // the entire structure is like the assembly language
        // It tries move the value in the preset registers
        // And use the simplest gramma to function well
        // Also its file is by fragments which is easy to reuse and locate
        /*
            Basic file structure:
            Line
            1       <version>
            2       <branch_name1> <x1> <y1> <branch_name2> <x2> <y2> <branch_name3> <x3> <y3> ...
            3       : <branch_name1> 
            4       <The_Exact_Code_For_the_particular_line_number>
            ...       ...
            15      </The_Exact_Code_For_the_particular_line_number>
            16      goto <branch_name>
            17      : <branch_name2>
            18      <The_Exact_Code_For_the_particular_line_number>
            ...        ...
            60      </The_Exact_Code_For_the_particular_line_number>
            61      endbr
            ...     ...

        */
        /*
            Example file: test.event
            1.0
            Func1 1 1 Func2 4 4 Func3 5 5
            : Func1
            mov $ 10 % power ; This is the first function branch, and it will be triggered when the player step on the position (1,1)
            mov $ 20 % blood
            mov $ 10 % max_power
            mov $ 20 % max_blood
            endbr
            : Func2
            multiply % power $ 2 % power ; This is the second function branch, and it will be triggered when the player step on the position (4,4)
            add % blood $ -5 % blood
            cmp % power $ 15
            jg Big
            endbr
            : Func3
            mov * a % gen_sym ; This is the third function branch, and it will be triggered when the player step on the position (5,5)
            mov * f % gen_sym
            mov $ 10 % gen_x
            mov $ 11 % gen_y
            mov $ 11 % gen_x
            mov $ 10 % gen_y
            goto Func1 ; There we call the function Func1
            : Big
            add $ -15 % power
            add $ 5 % blood
            endbr

        */
        /*
        Some Basic Gramma:
            mov  <source> <destination>  ; Just like the mov in the assembly
                <source>        : can be a symbol of the registers like : '% gen_x' , '% power' , '% x' , '% y' , '% gen_sym'
                                : also in some cases can be a direct number(single character/floating-point number) like :
                                             '$ 10' , '$ 20' , '$ 10.5' , '* a' , '* f' , '*  ' , '. 0.0' , '. 0.2' 
                                : And we will provide the support for the direct string like : '" Hello World, as I mentioned \" Ha Ha\" "'
                <destination>   : can be a symbol of the registers like : '% blood' , '% power' , '% x' , '% y'
            goto <branch_name>  ; Just to move the file pointer to certain branch
            add <source1> <source2> <destination>       ; Just to amplify the add operation, and can use the negative number to achieve sub operation
            multiply <source1> <source2> <destination>  ; Just to amplify the multiply operation
            divide <source1> <source2> <destination>    ; Just to amplify the divide operation, be aware of the result if it's not an integer
            endbr                                       ; Just to end the current event branch, and it is equivalent to goto 2/1
            cmp <source1> <source2>     ; Just to compare the two source, and set the cmp_flag in the variable_table 
                                          for the further use of the conditional jump
            je/jne/jg/jl <branch_name>  ; Just to achieve the conditional jump based on the cmp_flag
            call <sys_function_name> <arg1> <arg2>  ; Just to call the system function, 
                                                    ; and we will provide some basic system functions like : msg , random , etc.
        */
        inline void call(int i,variable_table& target,display_table& display){
            std::string branch_name = this->namemap[i];
            this->file.clear(); 
            this->file.seekg(this->branches[branch_name]);
            while(1){
                bool go = this->interpreter(target,display);
                if(!go){
                    break;
                }
            }
        }
        inline bool interpreter(variable_table& target,display_table& display){
            // This is the interpreter for the single line of the code in the event tree
            // It will interpret the code and execute the corresponding operation
            // For the sake of simplicity, we will not implement the full functionality of the event tree in this version
            // But we will implement some basic operations like mov , add , multiply , divide and goto
            std::string code;
            file >> code;
            bool is_getline = false;
            if(code == ":"){
                std::string label;
                file >> label;
                return true;
            }
            else if(code == "mov"){
                char sym1,sym2;
                file>>sym1;
                double value1;
                bool is_value1 = false;
                bool is_char1 = false;
                bool is_double1 = false;
                std::string reg1,reg2;
                if(sym1 == '$'){
                    file >> value1;
                    is_value1 = true;
                }
                else if(sym1 == '%'){
                    file>>reg1;
                    if(reg1 == "xmm1" || reg1 == "xmm0"){
                        is_double1 = true;
                    }
                }
                else if(sym1 == '*'){
                    file.ignore();
                    sym1 = (char)file.get();
                    is_char1 = true;
                    file.ignore();
                }
                else if(sym1 == '.'){
                    file>>value1;
                    is_double1 = true;
                    is_value1 = true;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                if(sym2 != '%'){
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>reg2;
                if(is_char1){
                    mov(target,sym1,reg2);
                }
                else if(is_value1 && !is_double1){
                    mov(target,(int)value1,reg2);
                }
                else if(is_value1 && is_double1){
                    mov(target,value1,reg2);
                }
                else if(!is_value1 && !is_double1){
                    mov(target,(int)get_number(target,reg1),reg2);
                }
                else if(!is_value1 && is_double1){
                    mov(target,get_number(target,reg1),reg2);
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                
            }
            else if(code == "add"){
                char sym1,sym2;
                file>>sym1;
                bool is_double = false;
                double value1;
                if(sym1 == '$'){
                    file >> value1;
                }
                else if(sym1 == '%'){
                    std::string reg1;
                    file>>reg1;
                    value1 = get_number(target,reg1);
                    if(reg1 == "xmm1" || reg1 == "xmm0"){
                        is_double = true;
                    }
                }
                else if(sym1 == '.'){
                    file>>value1;
                    is_double = true;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                double value2;
                if(sym2 == '$'){
                    file >> value2;
                }
                else if(sym2 == '%'){
                    std::string reg2;
                    file>>reg2;
                    value2 = get_number(target,reg2);
                    if(reg2 == "xmm1" || reg2 == "xmm0"){
                        is_double = true;
                    }
                }
                else if(sym2 == '.'){
                    file>>value2;
                    is_double = true;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file >> sym2;
                if(sym2 != '%'){
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                std::string dest;
                file>>dest;
                if(!is_double){
                    mov(target,(int)(value1+value2),dest);
                }
                else{
                    mov(target,value1+value2,dest);
                }
            }
            else if(code == "multiply"){
                char sym1,sym2;
                file>>sym1;
                bool is_double = false;
                double value1;
                if(sym1 == '$'){
                    file >> value1;
                }
                else if(sym1 == '%'){
                    std::string reg1;
                    file>>reg1;
                    value1 = get_number(target,reg1);
                    if(reg1 == "xmm1" || reg1 == "xmm0"){
                        is_double = true;
                    }
                }
                else if(sym1 == '.'){
                    file>>value1;
                    is_double = true;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                double value2;
                if(sym2 == '$'){
                    file >> value2;
                }
                else if(sym2 == '%'){
                    std::string reg2;
                    file>>reg2;
                    value2 = get_number(target,reg2);
                    if(reg2 == "xmm1" || reg2 == "xmm0"){
                        is_double = true;
                    }
                }
                else if(sym2 == '.'){
                    file>>value2;
                    is_double = true;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                if(sym2 != '%'){
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                std::string dest;
                file>>dest;
                if(!is_double){
                    mov(target,(int)(value1*value2),dest);
                }
                else{
                    mov(target,value1*value2,dest);
                }
            }
            else if(code == "cmp"){
                char sym1,sym2;
                file>>sym1;
                double value1;
                if(sym1 == '$'){
                    int p;
                    file >> p;
                    value1 = p;
                }
                else if(sym1 == '%'){
                    std::string reg1;
                    file>>reg1;
                    value1 = get_number(target,reg1);
                }
                else if(sym1 == '.'){
                    file>>value1;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                double value2;
                if(sym2 == '$'){
                    int p;
                    file>> p;
                    value2 = p;
                }
                else if(sym2 == '%'){
                    std::string reg2;
                    file>>reg2;
                    value2 = get_number(target,reg2);
                }
                else if(sym2 == '.'){
                    file>> value2;
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                if(value1 == value2){
                    target.cmp_flag = 0;
                }
                else if(value1 > value2){
                    target.cmp_flag = 1;
                }
                else if(value1 < value2){
                    target.cmp_flag = -1;
                }
            }
            else if(code == "je"){
                std::string label;
                file>>label;
                if(target.cmp_flag == 2){        }
                else{
                if(target.cmp_flag == 0){
                    auto it = this->branches.find(label);
                        if(it == this->branches.end()){
                            std::cerr<<"INVALID LABEL"<<std::endl;
                            return false;
                        }
                        file.clear();
                        file.seekg(it->second);
                        target.cmp_flag = 2; // Set the cmp_flag to 2 to indicate that the branch has been taken for the je instruction
                        return true;
                    }
                }
            }
            else if(code == "jne"){
                std::string label;
                file>>label;
                if(target.cmp_flag == 2){               }
                else{
                    if(target.cmp_flag != 0){
                        auto it = this->branches.find(label);
                        if(it == this->branches.end()){
                            std::cerr<<"INVALID LABEL"<<std::endl;
                            return false;
                        }
                        file.clear();
                        file.seekg(it->second);
                        target.cmp_flag = 2; // Set the cmp_flag to 2 to indicate that the branch has been taken for the jne instruction
                        return true;
                    }
                }
            }
            else if(code == "jg"){
                std::string label;
                file>>label;
                if(target.cmp_flag == 2){                }
                else{
                    if(target.cmp_flag == 1){
                        auto it = this->branches.find(label);
                        if(it == this->branches.end()){
                            std::cerr<<"INVALID LABEL"<<std::endl;
                            return false;
                        }
                        file.clear();
                        file.seekg(it->second);
                        target.cmp_flag = 2; // Set the cmp_flag to 2 to indicate that the branch has been taken for the jg instruction
                        return true;
                    }
                }
            }
            else if(code == "jl"){
                std::string label;
                file>>label;
                if(target.cmp_flag == 2){                }
                else{
                    if(target.cmp_flag == -1){
                        auto it = this->branches.find(label);
                        if(it == this->branches.end()){
                            std::cerr<<"INVALID LABEL"<<std::endl;
                            return false;
                        }
                        file.clear();
                        file.seekg(it->second);
                        target.cmp_flag = 2; // Set the cmp_flag to 2 to indicate that the branch has been taken for the jl instruction
                        return true;
                    }
                }
            }
            else if(code == "divide"){
                char sym1,sym2;
                file>>sym1;
                double value1;
                if(sym1 == '$'){
                    file >> value1;
                }
                else if(sym1 == '%'){
                    std::string reg1;
                    file>>reg1;
                    value1 = get_number(target,reg1);
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                double value2;
                if(sym2 == '$'){
                    file >> value2;
                }
                else if(sym2 == '%'){
                    std::string reg2;
                    file>>reg2;
                    value2 = get_number(target,reg2);
                }
                else{
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                file>>sym2;
                if(sym2 != '%'){
                    std::cerr<<"INVALID SYNTAX"<<std::endl;
                    return false;
                }
                std::string dest;
                file>>dest;
                if(value2 == 0){
                    std::cerr<<"DIVISION BY ZERO"<<std::endl;
                    return false;
                }
                if(dest == "xmm0" || dest == "xmm1"){
                    mov(target,value1/value2,dest);
                }
                else{
                    mov(target,(int)(value1/value2),dest);
                }
            }
            else if(code == "endbr"){
                this->file.clear();
                this->file.seekg(0,std::ios::beg);
                target.cmp_flag = 2; // Set the cmp_flag to 2 to prevent the further use of the conditional jump in the current branch
                return false;
            }
            else if(code == "goto"){
                std::string label;
                file>>label;
                auto it = this->branches.find(label);
                if(it == this->branches.end()){
                    std::cerr<<"INVALID LABEL"<<std::endl;
                    return false;
                }
                file.clear();
                file.seekg(it->second);
            }
            else if(code == "call"){
                std::string func_name;
                file>>func_name;
                if(func_name == "msg"){
                    file.ignore();
                    std::string msg;
                    std::getline(file,msg);
                    is_getline = true;
                    if(!msg.empty() && (msg.back() == '\r' || msg.back() == '\n')){
                        msg.pop_back();
                    }
                    target.msg.push_back(msg);
                    // Then we do something for this message , but we still haven't developed the entire system for the function calling

                }
                else if(func_name == "rand"){
                    char sym1,sym2;
                    file>>sym1;
                    double value1;
                    if(sym1 == '.'){
                        file >> value1;
                    }
                    else {
                        std::cerr<<"INVALID SYNTAX"<<std::endl;
                        return false;
                    }
                    file>>sym2;
                    if(sym2 == '%' && value1 == 1.0){
                        std::string reg2;
                        file>>reg2;
                        
                        // In my computer the RAND_MAX is 32767
                        // So the random value generated by rand() is between 0 and 32767
                        mov(target,rand(),reg2);
                    }
                    else if(sym2 == '%' && value1 == 0.5){
                        std::string reg2;
                        file>>reg2;
                        
                        double num = static_cast<double>(rand()) / RAND_MAX;
                        // In the floating-number version of the random function
                        // we will generate a random number between 0 and 1
                        mov(target,num,reg2);
                    }
                    else{
                        std::cerr<<"INVALID SYNTAX"<<std::endl;
                        return false;
                    }
                }
                else{
                    std::cerr<<"INVALID FUNCTION NAME"<<std::endl;
                    return false;
                }
            }
            else{
                std::cerr<<"INVALID CODE"<<std::endl;
                return false;
            }
            if(is_getline){
                return true;
            }
            std::streampos pos = file.tellg();
            bool is_end =false;
            int k;
            k = this->file.get();
            if(k == ' ' || k == '\t' || k == ';'){
                is_end = true;
            }
            else if(k == '\n' || k == EOF || k == '\r'){
                return true;
            }
            else{
                file.clear();
                file.seekg(pos);
                return true;
            }
            for(;;){
                k = this->file.get();
                if(k == '\n' || k == EOF){
                    break;
                }
            }
            return true;
        }
};

class engine{
    private:
        variable_table variable;
        display_table display;
        event_tree event;
    public:
        std::vector<int> last_msg_length;
        bool is_running;
        void set_up(const std::string& variable_file,const std::string& display_file,const std::string& event_file){
            read_variable_table(variable_file,variable);
            display.read(display_file);
            event.open(event_file);
            int col ,row;
            display.get_size(col,row);
            event.initialize_size(col,row);
            event.build();
        }
        inline void render(){
            display.display(variable);
        }
        inline void input_analysis(char input){
            int x = variable.x;
            int y = variable.y;
            if(input == 'w'){
                y--;
            }
            else if(input == 's'){
                y++;
            }
            else if(input == 'a'){
                x--;
            }
            else if(input == 'd'){
                x++;
            }
            else if(input == 'j'){
                const std::vector<std::vector<int>> dir = {{0,-1},{1,0},{0,1},{-1,0}};
                int col,row;
                display.get_size(col,row);
                if(x+dir[variable.direction][0] < 0 || x+dir[variable.direction][0] >= col || y+dir[variable.direction][1] < 0 || y+dir[variable.direction][1] >= row){
                    return;
                }
                this->trigger_check(x+dir[variable.direction][0],y+dir[variable.direction][1]);
            }
            else if((int)input == 27 ){
                this->is_running = false;
                return ;
            }

            bool move =display.teleport(x,y,variable);
            if(move){
                this->trigger_check(x,y);
            }
        }
        inline void trigger_check(int x,int y){
            if(this->event.trigger_table[y][x]!=0){
                int xx = this->variable.x;
                int yy = this->variable.y;
                this->event.call(this->event.trigger_table[y][x],this->variable,this->display);
                if(xx!=this->variable.x || yy!=this->variable.y){
                    bool move = display.teleport(this->variable.x,this->variable.y,variable);
                    
                    if(!move){

                        this->variable.x = xx;
                        this->variable.y = yy;
                    }
                }
            }
        }
        inline void generate(){
            if(variable.gen_x.size() == 0){
                return;
            }
            else{
                for(int i=0;i<variable.gen_x.size();i++){
                    display.generate(variable.gen_x[i],variable.gen_y[i],variable.gen_sym[i]);
                }
                variable.gen_x.clear();
                variable.gen_y.clear();
                variable.gen_sym.clear();
            }
        }
        inline void message(){
            if(variable.msg.empty()){
                return;
            }
            else{
                gotoxy(0,variable.sight*2+5);
                for(int i=0;i<last_msg_length.size();i++){
                    for(int j=0;j<last_msg_length[i];j++){
                        std::cout<<' ';
                    }
                    std::cout<<'\n';
                }
                this->last_msg_length.clear();
                gotoxy(0,variable.sight*2+5);
                for(int i=0;i<variable.msg.size();i++){
                    std::cout<<variable.msg[i]<<'\n';
                    this->last_msg_length.push_back(variable.msg[i].size());
                }
                variable.msg.clear();
                gotoxy(0,0);
            }
        }
        inline void run(int millisec_per_frame){ 
            HideCursor();                 // 隐藏光标，提升视觉体验
            srand(time(0));
            is_running = true;            // 确保运行标志为真;
            this->display.initial_teleport(variable.x,variable.y,variable); // 初始化玩家位置
            while (is_running) {
                render();                 // 渲染当前视野

                char ch = getcha();       // 非阻塞获取输入
                if (ch != 0) {
                    input_analysis(ch);         // 处理移动/交互（内部会触发事件）
                    generate();           // 应用事件中产生的地图修改
                    message();            // 显示事件中产生的消息（并清空消息队列）
                }
                
                                        //  60 FPS
                Sleep(millisec_per_frame);
            }

            event.close();                // 关闭事件脚本文件，释放资源
        }
};