// vidoplay.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include <iostream>
#include <vlc.h>
#include <windows.h>

std::string unicode2utf8(const std::wstring& strIn)
{
	std::string str;
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), LPSTR(str.c_str()), 0, NULL, NULL);
	str.resize(length + 1);
	::WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), strIn.size(), LPSTR(str.c_str()), length, NULL, NULL);
	return str;
}

int main()
{
	int argc = 1;
	char* argv[2];
	argv[0] = (char*)"--ignore-config";
	libvlc_instance_t* vlc_inst = libvlc_new(argc, argv);//多字节转 unicode 再转utf-8
	std::string path = unicode2utf8(L"file:///C:\\Users\\peng\\Desktop\\新建文件夹\\move.mp4");
	libvlc_media_t* media = libvlc_media_new_location(vlc_inst, path.c_str());
	libvlc_media_player_t* play = libvlc_media_player_new_from_media(media);
	int ret = libvlc_media_player_play(play);
	if (ret == -1)
	{
		std::cout << "error found" << std::endl;
		libvlc_media_player_release(play);
		libvlc_media_release(media);
		libvlc_release(vlc_inst);
		return 0;
	}
	Sleep(300);
	libvlc_time_t time = libvlc_media_player_get_length(play);
	printf("%02d:%02d:%02d \n", (int)(time / 3600000), (int)(time / 60000) % 60000, (int)time % 60000);
	int width = libvlc_video_get_width(play);
	std::cout << "width=" << width << std::endl;
	int height = libvlc_video_get_height(play);
	std::cout << "height=" << height << std::endl;

	getchar();
	libvlc_media_player_pause(play);
	getchar();
	libvlc_media_player_play(play);
	getchar();
	libvlc_media_player_stop(play);

	libvlc_media_player_release(play);
	libvlc_media_release(media);
	libvlc_release(vlc_inst);
	return 0;
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
