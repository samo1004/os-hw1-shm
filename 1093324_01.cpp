#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
using namespace std;
#define dfder "datafolder"
struct mydata {
    int attackflag = 0;
    bool csink=0;
    bool psink=0;
    bool hitc1=0;
    bool hitc2=0;
    bool hitp1=0;
    bool hitp2=0;
    int shoot[2]={0};
    int pboomcount=0;
    int cboomcount=0;
    pid_t cpid=0;
};
#define d_size sizeof(mydata) // wanted upload size

int main(int argc,char*argv[]) 
{
    int seed1=atoi(argv[1]);
    int seed2=atoi(argv[2]);
    int mode=atoi(argv[3]);
    if(mode!=0)
    {
        cout<<"mode error\n";
        return 0;
    }
    
    int fd = shm_open(dfder, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0) // shm fail
    {
        cout << "shm fail\n";
        shm_unlink(dfder); // unlink datafolder
        return 0;
    }
    // success
    ftruncate(fd, d_size);

    mydata *myd = (mydata *)mmap(0, d_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    pid_t pid = fork();

    if (pid < 0) // pid err
    {
        cout << "pid err\n";
        shm_unlink("dfder");
        return 0;
    }

    if (pid > 0) // parent process
    {
        pid_t pp=getpid();
        srand(seed1);
        //[2370 Parent]: Random Seed 5 
        cout<<"["<<pp<<" Parent]: Random Seed "<<seed1<<endl;
        myd = (mydata *)mmap(0, d_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        
        int a=rand()%4;//row
        int b=rand()%4;//column
        int boat1[2]={a,b};
        int boat2[2]={0,0};
        do
        {
            switch(rand()%4)
            {
                case 0:
                    boat2[0]=a-1;
                    boat2[1]=b;
                break;
                case 1:
                    boat2[0]=a;
                    boat2[1]=b+1;
                break;
                case 2:
                    boat2[0]=a-1;
                    boat2[1]=b;
                break;
                case 3:
                    boat2[0]=a;
                    boat2[1]=b-1;
                break;
            }
        } while (boat2[0]<0||boat2[1]<0||boat2[0]>3||boat2[1]>3);

        cout<<"["<<pp<<" parent]: The gunboat: "<<"("<<boat1[0]<<","<<boat1[1]<<")("<<boat2[0]<<","<<boat2[1]<<")\n";
        

        //cout << "p done and wait\n";
        while(myd->psink==0&&myd->csink==0)
        {
            int a=rand()%4;
            int b=rand()%4;
            myd->shoot[0]=a;
            myd->shoot[1]=b;
            cout<<"["<<pp<<" parent]: Bombing ("<<a<<","<<b<<")\n";
            myd->pboomcount++;
            myd->attackflag++;
            while (myd->attackflag == 1) 
            {
                myd = (mydata *)mmap(0, d_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // keep reading til children update
            }
            

            // cout<<"parent : child updated\n";
            if(myd->csink==0)
            {
                if((myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])||(myd->shoot[0]==boat2[0]&&myd->shoot[1]==boat2[1]))
                {
                    if(myd->hitp1==0&&myd->hitp2==0)
                    {
                        cout<<"["<<pp<<" Parent]: hit\n";
                        if(myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])
                        {
                            myd->hitp1=1;
                        }
                        else
                        {
                            myd->hitp2=1;
                        }
                    }
                    else if(myd->hitp1==1&&myd->hitp2==0)
                    {
                        if(myd->shoot[0]==boat2[0]&&myd->shoot[1]==boat2[1])
                        {
                            cout<<"["<<pp<<" Parent]: hit and sinking\n";
                            myd->psink=1;
                        }
                        else
                        {
                            cout<<"["<<pp<<" Parent]: missed\n";
                        }
                    }
                    else if(myd->hitp1==0&&myd->hitp2==1)
                    {
                        if(myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])
                        {
                            cout<<"["<<pp<<" Parent]: hit and sinking\n";
                            myd->psink=1;
                        }
                        else
                        {
                            cout<<"["<<pp<<" Parent]: missed\n";
                        }
                    }
                }
                else
                {
                    cout<<"["<<pp<<" Parent]: missed\n";
                }
            }
        }
        //cout<<"parent : child sinked\n";
        if(myd->csink)
        {
            cout<<"["<<pp<<" Parent]: "<<pp<<" wins with "<<myd->pboomcount<<" bombs\n";
        }
        else if(myd->psink)
        {
            cout<<"["<<pp<<" Parent]: "<<myd->cpid<<" wins with "<<myd->cboomcount<<" bombs\n";
        }
        close(fd);
        shm_unlink(dfder);
    }



    if (pid == 0) // child process/////////////////////////////////////////////////////////////////////////////////////////
    {
        pid_t cp=getpid();
        myd->cpid=cp;
        cout<<"["<<cp<<" Child]: Random Seed "<<seed2<<endl;
        srand(seed2);
        int a=rand()%4;//row
        int b=rand()%4;//column
        int boat1[2]={a,b};
        int boat2[2]={0,0};
        do
        {
            switch(rand()%4)
            {
                case 0:
                    boat2[0]=a-1;
                    boat2[1]=b;
                break;
                case 1:
                    boat2[0]=a;
                    boat2[1]=b+1;
                break;
                case 2:
                    boat2[0]=a-1;
                    boat2[1]=b;
                break;
                case 3:
                    boat2[0]=a;
                    boat2[1]=b-1;
                break;
            }
        } while (boat2[0]<0||boat2[1]<0||boat2[0]>3||boat2[1]>3);
        cout<<"["<<cp<<" child]: The gunboat: "<<"("<<boat1[0]<<","<<boat1[1]<<")("<<boat2[0]<<","<<boat2[1]<<")\n";
        
        
        while(myd->csink==0&&myd->psink==0)
        {
            while (myd->attackflag == 0)
            {
                myd = (mydata *)mmap(0, d_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            }

            //determine shooted(sinked)or not///////////////////////////////////////////////////////////////////////////
            if((myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])||(myd->shoot[0]==boat2[0]&&myd->shoot[1]==boat2[1]))
            {
                if(myd->hitc1==0&&myd->hitc2==0)//haven't shot
                {
                    cout<<"["<<cp<<" child]: hit\n";
                    if(myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])
                    {
                        myd->hitc1=1;
                    }
                    else
                    {
                        myd->hitc2=1;
                    }
                }
                else if(myd->hitc1==1&&myd->hitc2==0)
                {
                    if(myd->shoot[0]==boat2[0]&&myd->shoot[1]==boat2[1])
                    {
                        cout<<"["<<cp<<" child]: hit and sinking\n";
                        myd->csink=1;
                    }
                    else
                    {
                        cout<<"["<<cp<<" child]: missed\n";
                    }
                }
                else if(myd->hitc1==0&&myd->hitc2==1)
                {
                    if(myd->shoot[0]==boat1[0]&&myd->shoot[1]==boat1[1])
                    {
                        cout<<"["<<cp<<" child]: hit and sinking\n";
                        myd->csink=1;
                    }
                    else
                    {
                        cout<<"["<<cp<<" child]: missed\n";
                    }
                }
            }
            else
            {
                cout<<"["<<cp<<" child]: missed\n";
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //shoot
            if(myd->csink==0)
            {
                int a=rand()%4;
                int b=rand()%4;
                myd->shoot[0]=a;
                myd->shoot[1]=b;
                cout<<"["<<cp<<" Child]: Bombing ("<<a<<","<<b<<")\n";
                myd->cboomcount++;
            }

            myd->attackflag--; // change to parent stage
        }
        close(fd);
    }
    return 0;
}