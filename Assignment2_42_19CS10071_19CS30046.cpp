/**
 * @file shell.c
 * @author Anurat bhattacharya(19CS10071) Srijan Das(19CS30046)
 * @brief 
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <bits/stdc++.h>
#include <fcntl.h>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>

#define MAX_HIST 10000

/**
 * @brief Trie Data Structure
 * 
 */
struct node
{
    vector<node *> adj;
    int cnt;
    node()
    {
        adj.assign(256, NULL);
        cnt = 0;
    }
};
typedef struct node node;
struct Trie
{
    //set<string> st;//set of inserted strings for checking exact match
    node *root;
    vector<string> ans;
    string stk = "";
    Trie() : root(NULL) { root = new node(); }
    void insert(string str)
    {
        // st.insert(str);

        int lv = 0;
        node *cur = root;
        while (lv < str.size() && cur != NULL)
        {
            if (cur->adj[str[lv]] == NULL)
            {
                cur->adj[str[lv]] = new node();
            }
            cur = cur->adj[str[lv]];
            lv++;
            if (lv == str.size())
                cur->cnt++;
        }
    }
    void dfs(node *head)
    {
        if (head->cnt > 0)
            ans.push_back(stk);
        for (int i = 0; i < 256; i++)
            if (head->adj[i] != NULL)
            {
                stk.push_back(i);
                dfs(head->adj[i]);
                stk.pop_back();
            }
    }
    int search(string str)
    {
        int lv = 0;
        string ss = "";

        node *cur = root;
        while (lv < str.size() && cur->adj[str[lv]] != NULL)
        {
            cur = cur->adj[str[lv]];
            ss.push_back(str[lv]);
            lv++;
        }
        if (lv < str.size())
        {
            return -1;
        }
        ans.clear();
        stk = ss;
        dfs(cur);
        return 1;
    }
};
typedef struct Trie Trie;

string pwd = ".";
struct termios t;
/**
 * @brief to parse the line based on delimiters
 *          Idea : First do with delim = "|" to breakup into piped parts
 *                  and then handle each part  
 * @param buf :Input string
 * @param arr : Array into which stoting the tokens
 * @param ln :Number of tokens
 * @param delim :delimiter
 */
void parse_command(char *buf, char arr[][10000], int *ln, char delim)
{
    
    int j = 0;
    int state = 1;
    int k = 0;
    for(int i=0;buf[i]!='\0';i++){
        if(buf[i]=='\"')    {
            state= 1-state;
            continue;
        }
        if(buf[i]==delim){
            if(state){
                if(k>0){
                    arr[j][k]='\0';
                    k = 0;
                    j++;
                }
                while((buf[i]!='\0')&&(buf[i]==delim))    i++;
                i--;
                
            }else{
                arr[j][k++] = buf[i];
            }
        }else   arr[j][k++] = buf[i];
        
        
    }
    arr[j][k] = '\0';
    *ln = j+1;
}


void parse_command_(char *buf, char arr[][10000], int *ln, char *delim)
{
    char str[50];
    strcpy(str,buf);
    int i = 0;
    *ln = 0;
    char *cur = strtok(str, delim);
    while (cur != NULL)
    {
        strcpy(arr[i], cur);
        cur = strtok(NULL, delim);
        (*ln)++;
        i++;
    }
}
//GLOBALS
int INTERRUPT;
char buf[100];
int flg;
vector<int> children;//to kill children
void handleInterrupt(int sg)
{
    
    for(auto pid:children){
        kill(pid,SIGINT);
        string filename = ".temp."+to_string(pid)+".txt";
        remove(filename.c_str());
    }
    //printf("Shell Running...\n");
    flg =0;
    signal(SIGINT, handleInterrupt);
    fflush(stdout);
}
int curChld;
void handleInterruptC(int sg)
{
    exit(0);
}
void handleMoveBck(int sg)
{
    printf("\nMoving Process  to Background\n");
    signal(SIGTSTP, handleMoveBck);
    kill(curChld,SIGCONT);
    fflush(stdout);
}
void handleMoveBckC(int sg)
{
    //printf("Moving to Back\n");
    //printf("Process moved to Background\n");
    //signal(SIGTSTP,handleMoveBckC );
    fflush(stdout);
}
int fnd(string A, string B)
{
    int ans = 0;
    for (int i = 0; i < A.size(); i++)
    {
        int j = i;
        while ((j - i) < B.size() && j < A.size() && A[j] == B[j - i])
            j++;
        ans = max(ans, j - i);
    }
    return ans;
}
char curCmd[100][10000];
/**
 * @brief : handle current command and return output as a string
 * 
 * @param curr 
 * @return char* 
 */
int handleCommand(char *curr, int input, int ret, deque<string> &dq)
{
    int in = 0, out = 0;
    int i;
    char inp_file[20], out_file[20];
    for (i = 0; i < strlen(curr); i++)
    {
        if (curr[i] == '<')
            in = i;
        else if (curr[i] == '>')
            out = i;
    }
    int t1,t2;
    t1 = in;
    t2 = out;
    int willWait = 1;
    if (in != 0)
    {
        i = 0;
        while (curr[++in] == ' ')
            ; // skipping space

        // copying input file name in inp_file
        while ((curr[in] != ' ') && (curr[in] != '\0'))
        {
            inp_file[i] = curr[in];
            i++;
            in++;
        }
        inp_file[i] = '\0';

        input = open(inp_file, O_RDONLY);
        if(input<0){
            printf("%s : Error opening File\n",inp_file);
            return 1;
        }
       // dup2(input, STDIN_FILENO);
    }
    if (out != 0)
    {
        i = 0;
        while (curr[++out] == ' ')
            ;
        while ((curr[out] != ' ') && (curr[out] != '\0'))
        {
            out_file[i] = curr[out];
            i++;
            out++;
        }
        out_file[i] = '\0';
        ret = open(out_file, O_WRONLY| O_CREAT|O_TRUNC,0666);
        //dup2(ret, STDOUT_FILENO);
    }
    in = t1;
    out = t2;
   
   
    int j = 0;
    string chk(curr);
    for(int i=0;curr[i]!='\0';i++){
        
        if(i>0&&i==in){
            i++;
            while(curr[i]==' ')  i++;
            while(curr[i]!=' '&&curr[i]!='\0') i++;
        }
        if(i>0&&i==out){
            i++;
            while(curr[i]==' ')  i++;
            while(curr[i]!=' '&&curr[i]!='\0') i++;
        }
        curr[j]  =curr[i];
        j++;
    }
    curr[j] = '\0';
    //if input == NULL no such input any such input ingrained in the comnd
    // else input = input
    int ln;
    //qa b d g < k > nk
    chk = string(curr);
    parse_command(curr, curCmd, &ln, ' ');
    
    if (strcmp(curCmd[ln - 1], "&") == 0)
    {
        willWait = 0;
    }
    //first assume input = NULL
    // printf("%s\n",curr);
    chk = string(curr);
    int ind = 0;
    if (strcmp(curCmd[0], "history") == 0)
    {   
        int prein = dup(STDIN_FILENO);
        int preout = dup(STDOUT_FILENO);
        if(input != STDIN_FILENO)   dup2(input,STDIN_FILENO);
        if(ret != STDOUT_FILENO)    dup2(ret,STDOUT_FILENO);
        for (auto itr : dq)
        {
            cout << ++ind << " " << itr << "\n";
        }
        if(input != STDIN_FILENO)   close(input);
        if(ret != STDOUT_FILENO)    close(ret);
        dup2(prein,STDIN_FILENO);
        dup2(preout,STDOUT_FILENO);
        return 1;
    }
    if(strcmp(curCmd[0],"cd")==0){
        if(chdir(curCmd[1])<0){
            printf("Error in changing directory\n");
        }
        return 1;
    }
    if(strcmp(curCmd[0],"multiwatch")==0){
         int prein = dup(STDIN_FILENO);
        int preout = dup(STDOUT_FILENO);
        if(input != STDIN_FILENO)   dup2(input,STDIN_FILENO);
        if(ret != STDOUT_FILENO)    dup2(ret,STDOUT_FILENO);
        flg = 1;
        children.clear();
        signal(SIGINT,handleInterrupt);
        //first parse
        char temp[100];
        char Cmds[20][10000];
        string db(curr);
        strcpy(temp,curr);
        //cout<<temp<<"\n";
        parse_command_(temp, Cmds, &ln, "[");
        //cout<<Cmds[0]<<"\n"<<Cmds[1]<<"\n";
        strcpy(temp,Cmds[1]);
        
        parse_command_(temp, Cmds, &ln, "]");
        strcpy(temp,Cmds[0]);
        
        parse_command_(temp, Cmds, &ln, ",");
        /*
        for(int i = 0;i<ln;i++){
            
            strcpy(temp,Cmds[i]);
            int l = 0;
            int r = strlen(temp);
            while(temp[l]!='\"')    l++;
            l++;
            while(temp[r]!='\"')    r--;
            r--;
            //string pp(temp);
            //pp = pp.substr(l,r-l+1);
            //strcpy(Cmds[i],pp.c_str());


        }*/
        //for(int i=0;i<ln;i++)   cout<<Cmds[i]<<"\n";
        //create files for each process
        vector<int> op;

        
        
        for(int i=0;i<ln;i++){
            int val = fork();
            
            //create temp file
            

            if(val==0){
                //child
                pid_t id = getpid();
                int curfd = open((".temp."+to_string(id)+".txt").c_str(),O_WRONLY|O_CREAT|O_TRUNC,0666);
                handleCommand(Cmds[i],STDIN_FILENO,curfd,dq);
                exit(0);

            }else{
                //parent
                int curfd = open((".temp."+to_string(val)+".txt").c_str(),O_RDONLY|O_CREAT,0666);
                op.push_back(curfd);
                children.push_back(val);
               // waitpid(val, NULL, WUNTRACED);
            }
        }
        fd_set master;
        FD_ZERO(&master);
        int mx = 0;
        for(auto itr:op)    {
            FD_SET(itr,&master);
            mx = max(mx,itr);
        }
        time_t t;
        while(1){
            if(!flg)    break;
            int status = select(mx+1,&master,NULL,NULL,NULL);
            if(status>=1){
                for(int i=0;i<op.size();i++){
                    if(FD_ISSET(op[i],&master)){
                        char ch[100];
                        if(read(op[i],ch,99)==0)    continue;
                        time(&t);
                        cout<<Cmds[i]<<", "<<asctime(localtime(&t))<<"\n";
                        
                        cout<<"<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-\n";
                        cout<<ch;
                        while(read(op[i],ch,99))    cout<<ch;
                        cout<<"->->->->->->->->->->->->->->->->->->->\n";

                    }
                }
            }else if(status<0){
                printf("Error on select\n");
                break;
            }
            
            //break;
        }
        if(input != STDIN_FILENO)   close(input);
        if(ret != STDOUT_FILENO)    close(ret);
        dup2(prein,STDIN_FILENO);
        dup2(preout,STDOUT_FILENO);
        return 1;
    }
    int val = fork();
    curChld = val;
    t.c_lflag |= ECHO;
    t.c_cflag |=ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    if (val == 0)
    {
        
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        // printf("%s %s\n",curCmd[0],curCmd[1]);
        // "ls -v -l 5 6 "
        char *_curCmd[100];
        for (int i = 0; i < ln; i++)
            _curCmd[i] = curCmd[i];
        //for (int i = 0; i<ln; i++)
        //    printf("%s\n", curCmd[i]);
        _curCmd[ln++] = NULL;
        if(input != STDIN_FILENO)   dup2(input,STDIN_FILENO);
        if(ret != STDOUT_FILENO)    dup2(ret,STDOUT_FILENO);
        //execlp("ls","ls","-v","-l","5","6")
        // execlp(curCmd[0],curCmd)
        //printf("Hello\n");
        //char temp[100] = "/bin/";
        //strcat(temp,(const char*)curCmd[0]);

        // printf("%s\n", temp);
        // for (int i = 0; _curCmd[i] != NULL; i++)
        //    printf("%s\n", _curCmd[i]);

        // printf("Hello %s\n",curCmd[0]);
        
        if (execvp(curCmd[0], &_curCmd[0]) < 0)
        {
            printf("error\n");
        }
        
        exit(0);
        return 0;
    }
    else
    {
        signal(SIGTSTP,handleMoveBck);

        if (willWait){
            waitpid(val, NULL, WUNTRACED|WCONTINUED);
            //cout<<"Waiting\n";
            //wait(NULL);
            if(input!=STDIN_FILENO)    close(input);
            if(ret!=STDOUT_FILENO)   close(ret); 
        }
        else
        {
            printf("Process moved to Background\n");
        }
    }
    t.c_cflag &=~ICANON;
    t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    return val;
    // computation
}

int readline(char *buf, deque<string> &dq)
{
    int i = 0;
    int flg = 0;
    while (1)
    {
        char ch = cin.get();
        if (flg)
        {
            flg = 0;
            continue;
        }
        //putchar(ch);
        //cout<<ch<<"\n";
        if (ch == -1)
        {
            perror("Error in reading\n");
            exit(0);
        }
        if (ch == '\n' && (!flg))
        {
            cout.put(ch);
            break;
        }
        else if (!flg)
        {
            flg = 0;
            if (ch == 18)
            {
                //handle refresh
                printf("\nEnter Search string: ");
                char cc[100];
                t.c_lflag |= ECHO;
                tcsetattr(STDIN_FILENO, TCSANOW, &t);
                cin.getline(cc, 100);
                t.c_lflag &= ~ECHO;
                tcsetattr(STDIN_FILENO, TCSANOW, &t);
                string str(cc);
                int mx = 0;
                set<pair<string, int>> st;
                int ind = 0;
                for (auto itr : dq)
                {
                    if (itr == str)
                    {
                        mx = str.size();
                        st.clear();
                        st.insert({itr, ind});
                    }
                    int len = fnd(itr, str);
                    if (len > mx)
                    {
                        mx = len;
                        st.clear();
                    }
                    if (len == mx && len > 2)
                        st.insert(make_pair(itr, ind));
                    ind++;
                }
                if (st.size() == 0)
                {
                    printf("Sorry No match for search item in history\n");
                }
                else if (st.begin()->first == str)
                {
                    //exact match
                    printf("\nIndex      Command\n");
                    printf("%d      %s\n", st.begin()->second, st.begin()->first.c_str());
                }
                else
                {
                    printf("Index      Command\n");

                    for (auto it : st)
                    {
                        printf("%d      %s\n", it.second, it.first.c_str());
                    }
                }
                return 0;
            }
            else if (ch == '\t')
            {
                Trie ds;
                DIR *d = opendir(pwd.c_str());
                struct dirent *cur;

                while ((cur = readdir(d)) != NULL)
                {
                    //cout<<cur->d_name<<"\n";
                    ds.insert(string(cur->d_name));
                }
                closedir(d);
                char str[100];
                int ind = i - 1;
                while (ind >= 0 && buf[ind] != ' ' && buf[ind] != '<' && buf[ind] != '>' && buf[ind] != '/')
                {
                    ind--;
                }
                int sz = i - ind - 1;
                if (sz >= 0)
                {
                    strncpy(str, buf + ind + 1, i - ind - 1);
                    str[sz] = '\0';
                    int val;
                    if ((val = ds.search(string(str))) > 0)
                    {
                        if (ds.ans.size() == 1)
                        {
                            string fl = ds.ans[0];
                            for (int j = i; j - i + sz < fl.size(); j++)
                            {
                                buf[j] = fl[j - i + sz];
                                cout << fl[j - i + sz];
                            }
                            i += (fl.size() - sz);
                        }
                        else
                        {
                            cout << "Please choose:\n";
                            for (int ind = 0; ind < ds.ans.size(); ind++)
                            {
                                cout << ind << " " << ds.ans[ind] << "\n";
                            }
                            cout << "Enter Choice: ";
                            t.c_lflag |= ECHO;
                            tcsetattr(STDIN_FILENO, TCSANOW, &t);
                            int ind;
                            cin >> ind;
                            t.c_lflag &= ~ECHO;
                            tcsetattr(STDIN_FILENO, TCSANOW, &t);
                            string fl = ds.ans[ind];
                            cout << ">>> ";
                            for (int j = 0; j < i; j++)
                                cout << buf[j];
                            for (int j = i; j - i + sz < fl.size(); j++)
                            {
                                buf[j] = fl[j - i + sz];
                                cout << fl[j - i + sz];
                            }
                            flg = 1;
                            i += (fl.size() - sz);
                        }
                    }
                }
            }
            else
            {
                cout.put(ch);
                buf[i++] = ch;
            }
        }
    }
    buf[i] = '\0';
    return 1;
}
/**
 * @brief main
 * 
 * @return int 
 */
int main()
{

    //Key presses

    
    tcgetattr(STDIN_FILENO, &t);

    signal(SIGTSTP, handleMoveBck);
    char tokens[20][10000];

    fstream fin(".shellrc", ios::in);
    int lines = 0;
    string cur;
    deque<string> dq;
    char hist[100];
    while (fin.getline(hist,100))
    {
        cur = string(hist);
        lines++;
        dq.push_back(cur);
    }
    //Now work with the dq and at the end rewrite the file

    //fd_set master;
    //FD_ZERO(&master);
    //int keyfd = op
    while (1)
    {
        t.c_lflag &= ~ICANON;       
        t.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
       
        INTERRUPT = 0;
        signal(SIGINT, SIG_IGN);

        printf(">>> ");

        memset(buf, '\0', sizeof buf);

        /*******************************/
        if (!readline(buf, dq))
            continue;
        //cout<<buf<<"\n";
        //fgets(buf,sizeof buf,stdin);

        int len = strlen(buf);

        if (strcmp(buf, "") == 0)
            continue;
        if (dq.size() == MAX_HIST)
            dq.pop_front();
        dq.push_back(string(buf));
        /***********************/
        // printf("---%s----\n", buf);
        if (strcmp(buf, "") == 0)
        {
            //  printf("Hi\n");
            continue;
        }
        if (strcmp(buf, "quit") == 0)
            break;
        int ln;
        //char *prev = NULL;
        parse_command(buf, tokens, &ln, '|');
        vector<int*> p(ln,NULL);
        for(auto &itr:p)    itr = new int[2];  
        int flg = 1;
        int infd,outfd;
        for(auto &itr:p)    pipe(itr);
        for (int i = 0; i < ln; i++)
        {
            int curlen;
            // parse_command(tokens[i],curCmd,curlen," ");
            char *res;
            if(i>0) infd = p[i-1][0];
            else    infd = STDIN_FILENO;
            if(i+1<ln)  outfd = p[i][1];
            else    outfd = STDOUT_FILENO;
           // printf("Exec -> %s\n",tokens[i]);
            if (handleCommand(tokens[i], infd, outfd, dq) == 0)
                flg = 0;
        }

        if (flg == 0)
            break;
    }
    //Rewrite file
    fstream fout(".shellrc", ios::out);
    for (auto itr : dq)
        fout << itr << "\n";
    t.c_lflag |= ICANON;
    t.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSADRAIN, &t);
    return 0;
}
