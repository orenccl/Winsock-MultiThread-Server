# MintServer
###### tags: `C++` `Winsock` `Multi-Client Server`  
![](https://img.shields.io/github/license/orenccl/Winsock-Multi-Client-Server)  

## The Plan  
Learn how to careat a Winsock-Multi-Client-Server, and record how I learn it.  

## Why using C++?  
### Pros  
* High performance.  
* Most of software, Engine, API etc... is create by C++.  
* Control Memory Manually to enhance efficiency and stability.  
* Create wheel by myself.  

### Cons  
* If not use pointer, array, list etc... carefully, may cause a fatal error.  
* Control Memory Manually is dangerous.  
* C++ has no memory security check.  
* A lot of code is needed compared to other languages.  
* Hard to debug a big project.  
* Not easy for beginner.  

## Prior Knowledge
* Basic of C language  
* Pointer  
* Class  
* Inheritance  
* Polymorphism  
* Template  
* Data structure and Algorithm  

Here is recommanded toturial for C++ learning :  
[Learn C++ Programming -Beginner to Advance- Deep Dive in C++](https://www.udemy.com/share/101WveBUUSeVpXRHo=/)  
[Mastering Data Structures & Algorithms using C and C++](https://www.udemy.com/share/101WoeBUUSeVpXRHo=/)  
[The Cherno C++ series](https://www.youtube.com/watch?v=18c3MTX0PK0&list=PLlrATfBNZ98dudnM48yfGUldqGD0S4FFb)  

## Popular Game Engine's programming language  
* Unity Engine :   
  Core : C++  
  Application : C#  
  
* Unreal Engine :  
  All build on C++  
  
* Photon Engine :  
  Server Core : C++  
  Server Application : C#  
  Client Application : Various of popular language.  

## A Game Engine's Structure  
### Main Program is usually a .exe file.  
### Impltment with .dll files, for example :  
* Comman -- Commanly use function, API.  
* Net -- Network communication ( winsock / IOCP )  
* SQL -- Data library ( mySQL / MSSQL )  
* Gameplay -- Gaming Logic  
* ToolUI -- D3D / OpenGL  
* Image -- Image format ( bmp / tga / png / jpg )  
* GUI -- Game UI  

## Game Engine Framework History
```flow
st=>start: Object-Oriented Designed
e=>end: ECS + Job System
op=>operation: Component-Based Design

st->op->e
```

## Environment  
Compiler : VS2019  
Platform : Window x64  

## Main Feature  
* Multi-Threading  
* CPU cache hit  
* Less OOP ( Object-Oriented Design )  
* More Component-Based, Data-Oriented Design  

## Future Plan  
* Decentralized-Server framework  
* Concurrency and Parallelism  
* WindorAPI IOCP  

## Toturial
[Toturial 1 - TCP Server](https://hackmd.io/@Jun/Hy_oLrHsr)  

[Toturial 2 - Memory](https://hackmd.io/@Jun/HJnPquHsS)  
