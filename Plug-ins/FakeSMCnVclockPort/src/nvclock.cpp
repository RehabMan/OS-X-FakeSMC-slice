/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * Copyright(C) 2001-2008 Roderick Colenbrander
 *
 * site: http://NVClock.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

//#include <unistd.h>
//#include <getopt.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include "ctype.h"
//#include "config.h"
#include <nvclock.h>
#ifdef HAVE_NVCONTROL
#include <nvcontrol.h>
#endif

bool isxdigit(char c) {
	if (((c>='0')&&(c<='9'))||((c>='a')&&(c<='f'))||((c>='A')&&(c<='F')))
		return true;
	return false;
}

/*static struct option long_options[] =
{
	{"memclk", 1, 0, 'm'},
	{"nvclk", 1, 0, 'n'},
	{"card", 1, 0, 'c'},
	{"backend", 1, 0, 'b'},
	{"fanspeed", 1, 0, 'F'},
	{"punit", 1, 0, 'P'},
	{"devicid", 1, 0, 'Q'},
	{"smartdimmer", 1, 0, 'S'},
	{"vunit", 1, 0, 'V'},
	{"force", 0, 0, 'f'},
	{"assign", 1, 0, 'a'},
	{"query", 1, 0, 'q'},
	{"list", 0, 0, 'l'},
	{"xdisplay", 1, 0, 'x'},
	{"info", 0, 0, 'i'},
	{"temperature", 0, 0, 'T'},
	{"reset", 0, 0, 'r'},
	{"speeds", 0, 0, 's'},
	{"debug", 0, 0, 'd'},
	{"Debug", 0, 0, 'D'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'v'},
	{0, 0, 0, 0}
};
*/
int usage()
{
	printf("NVClock v0.8 (Beta4)\n\n");
	printf("Using NVClock you can overclock your Nvidia videocard under Linux and FreeBSD.\nUse this program at your own risk, because it can damage your system!\n\n");
	printf("Usage: ./nvclock [options]\n\n");
	printf("Overclock options:\n");
	printf("   -b  --backend backend\tBackend to use: coolbits/coolbits2d/coolbits3d/lowlevel (NV3X/NV4X/NV5X only)\n");
	printf("   -m  --memclk speed\t\tMemory clock in MHz\n");
	printf("   -n  --nvclk speed\t\tGPU clock in MHz\n");
	printf("   -r  --reset\t\t\tRestore the original speeds\n");
	printf("   -s  --speeds\t\t\tPrint current speeds in MHz\n");
	printf("   -d  --debug\t\t\tEnable/Disable clock related debug info\n");
	printf("Hardware options:\n");
	printf("   -c  --card number\t\tNumber of the card to use\n");
	printf("   -D  --Debug\t\t\tPrint detailed debug information\n");
	printf("   -f  --force\t\t\tForce support for disbled hardware\n");
	printf("   -F  --fanspeed speed\t\tAdjust the fanspeed; speed is a value between 10 and 100, a delta +10/-10 or 'auto'\n");
	printf("   -P  --punit mask\t\tActivate extra pixel pipelines. (NV4X only)\n");
	printf("   -Q  --deviceid digit\t\tAdjust the last digit of the pci id.\n");
	printf("   -S  --smartdimmer level\tAdjust brightness of the backlight; level is a value between 15 and 100 or a delta like +10/-10.\n");
	printf("   -T  --temperature\t\tShow the GPU temperatures.\n");
	printf("   -V  --vunit mask\t\tActivate extra vertex pipelines. (NV4X only)\n");
	printf("   -i  --info\t\t\tShow detailed card info.\n");
#ifdef HAVE_NVCONTROL
	printf("NVControl options:\n");
	printf("   -a  --assign\t\t\tSet an option to a value: -a fsaa=4 or -a vibrance[crt-0]\n");
	printf("   -q  --query\t\t\tGet the value for an option: -q fsaa or -q vibrance[crt-0]=63\n");
	printf("   -l  --list\t\t\tShow all available options\n");
	printf("   -x  --xdisplay\t\tChoose another X display\n");
#endif
	printf("Other options:\n");
	printf("   -h  --help\t\t\tShow this help info\n");
	return 0;
}


#ifdef HAVE_NVCONTROL
enum
{
	QUERY_VALUE = 0,
	ASSIGN_VALUE
};

/* Structure in which we'll temporarily store the requested opengl options */
typedef struct
{
	int option;
	int value;
	int mask;
} GLTmp;

GLTmp *query_list = NULL;
int query_size = 0;
GLTmp *assign_list = NULL;
int assign_size = 0;

char *mask_to_device(int mask)
{
	char *res;
	int i=0;
	if(mask & NV_CRT)
	{
		res = calloc(6, sizeof(char));
		while((mask & (1<<i)) == 0)
			i++;

		sprintf(res, "crt-%d", i);
	}
	else if(mask & NV_TV)
	{
		res = calloc(5, sizeof(char));
		while((mask & (1<<i)) == 0)
			i++;

		sprintf(res, "tv-%d", i-8);
	}
	else if(mask & NV_DFP)
	{
		res = calloc(6, sizeof(char));
		while((mask & (1<<i)) == 0)
			i++;

		sprintf(res, "dfp-%d", i-16);
	}
	return res;
}


/* Function for parsing opengl options passed to nvclock using -a option=X / -q option */
void parse_gl_cmdline_option(char *option, int task)
{
	NVOptionList *lst = option_list;
	for(; lst->name != NULL; lst++)
	{
		if(strncmp(option, lst->name, lst->size) == 0)
		{
			int mask=0;
			short offset = lst->size;

			if(option[offset] == '[')
			{
				char *buf = &option[offset+1];
				if(strncmp("crt", buf, 3) == 0)
				{
					if((buf[3] == '-') && isdigit(buf[4]))
						mask = CRT_0 << (buf[4] - 48);
					else
						mask = CRT_0;
				}
				else if(strncmp("dfp", buf, 3) == 0)
				{
					if((buf[3] == '-') && isdigit(buf[4]))
						mask = DFP_0 << (buf[4] - 48);
					else
						mask = DFP_0;
				}
				else if(strncmp("tv", buf, 2) == 0)
				{
					if((buf[2] == '-') && isdigit(buf[3]))
						mask = TV_0 << (buf[3] - 48);
					else
						mask = TV_0;
				}

				while(option[offset] != '=')
					offset++;
			}

			if(task == QUERY_VALUE)
			{
				if(query_list == NULL)
				{
					query_list = calloc(1, sizeof(GLTmp));
				}
				else
				{
					query_list = realloc(query_list, sizeof(GLTmp)*(query_size+1));
				}
				query_list[query_size].mask = mask;
				query_list[query_size].option = lst->option;
				query_size++;
			}
			else
			{
				if(option[offset] != '=')
				{
					printf("Incorrect option: '%s'\n", option);
				}
				else
				{
					int value;
					option[offset] = '\0';
					option += offset + 1;
					value = atoi(option);

					if(assign_list == NULL)
					{
						assign_list = calloc(1, sizeof(GLTmp));
					}
					else
					{
						assign_list = realloc(assign_list, (assign_size + 1)*sizeof(GLTmp));
					}

					assign_list[assign_size].mask = mask;
					assign_list[assign_size].option = lst->option;
					assign_list[assign_size].value = value;
					assign_size++;
				}
			}
			return;
		}
	}
	printf("Unknown option '%s'\n", option);
}


void ShowGlAttributes(Display *dpy)
{
	int val = 0;
	int mask;
	int i=0;
	NVOptionList *lst = option_list;

	printf("Available OpenGL options:\n");
	for(; lst->name != NULL; lst++)
	{
		/* For now use NVGetAttribute to see if an option is supported as
		/  NVGetValidAttributeValues is broken in current nvidia drivers.
		/  For some unsupported options it returns supported.
		/  Note that by keeping the disp_mask 0 we don't get screen-specific options.
		*/
		if(!NVGetAttribute(dpy, 0, 0, lst->option, &val))
		{
			/* Option isn't supported on this card */
			continue;
		}

		printf(" %s", lst->name);
	}
	printf("\n");

	/* Get a mask of enabled displays to check what devices we have and what options they support */
	NVGetAttribute(dpy, 0, 0, NV_ENABLED_DISPLAYS, &mask);

	/* Get first screen from the mask */
	while(!((mask>>i) & 0x1))
		i++;

	while(mask>>i && i < 32)
	{
		printf("Display options for %s:\n", mask_to_device((mask & (1<<i))));

		lst = option_list;
		for(; lst->name != NULL; lst++)
		{
			int val;
			if(!NVGetAttribute(dpy, 0, mask & (1<<i), lst->option, &val))
			{
				/* Option isn't supported on this card */
				continue;
			}
			if((mask & (1<<i)) & lst->flags)
				printf(" %s", lst->name);
		}
		printf("\n");

		i++;
		/* Get the next screen */
		while(((mask>>i) & 0x1)==0)
		{
			i++;
		}
	}
}


void GetAttribute(Display *dpy, int screen, int disp_mask, int option)
{
	NVOptionList *opt = nvcontrol_lookup_option(option);
	validated *res;
	int val=0;

	/* When flags is set then we are dealing with a display specific option.
	/  Because of this we need to check if the given mask is valid.
	*/
	if(opt->flags)
	{
		int mask;
		NVGetAttribute(dpy, 0, 0, NV_ENABLED_DISPLAYS, &mask);

		if(disp_mask == 0)
		{
			printf("Error: Option '%s' needs a display.\n", opt->description);
			return;
		}

		if((disp_mask & mask) == 0)
		{
			printf("Error: Invalid display: %s.\n", mask_to_device(disp_mask));
			return;
		}
	}

	if(!NVGetAttribute(dpy, 0, disp_mask, option, &val))
	{
		printf("Error: Option '%s' isn't supported on this card.\n", opt->description);
		return;
	}
	else
		printf("Current value for option '%s': %d\n", opt->description, val);

	NVGetValidAttributeValues(dpy, screen, disp_mask, option, &res);
	/* boolean */
	if(res->type == 3)
	{
		printf("Supported values for this option are: 0 and 1\n");
	}
	/* range */
	else if(res->type == 4)
	{
		printf("Supported values lie in the range from %d to %d\n", res->data1, res->data2);
	}
	/* bitmask */
	else if(res->type == 5)
	{
		int i;
		printf("Supported values are:");
		for(i=0; i <= 13; i++)
		{
			if((res->data1 >> i) & 0x1)
			{
				printf(" %d", i);
			}
		}
		printf("\n");
	}

}


void SetAttribute(Display *dpy, int screen, int disp_mask, int option, int value)
{
	int val=0;
	NVOptionList *opt = nvcontrol_lookup_option(option);
	validated *res;

	/* When flags is set then we are dealing with a display specific option.
	/  Because of this we need to check if the given mask is valid.
	*/
	if(opt->flags)
	{
		int mask;
		NVGetAttribute(dpy, 0, 0, NV_ENABLED_DISPLAYS, &mask);

		if(disp_mask == 0)
		{
			printf("Error: Option '%s' needs a display.\n", opt->name);
			return;
		}

		if((disp_mask & mask) == 0)
		{
			printf("Error: Invalid display: %s.\n", mask_to_device(disp_mask));
			return;
		}

		if(!NVGetAttribute(dpy, screen, disp_mask, option, &val))
		{
			printf("Option '%s' isn't supported on display '%s'\n", opt->name, mask_to_device(disp_mask));
			return;
		}
	}

	/* First use NVGetAttribute to see if the option is supported because NVGetValidAttributeValues is broken. */
	if(!NVGetAttribute(dpy, screen, disp_mask, option, &val))
	{
		printf("Option '%s' isn't supported on this card\n", opt->name);
		return;
	}
	NVGetValidAttributeValues(dpy, screen, disp_mask, option, &res);

	/* integer */
	if(res->type == 1)
	{
		/* No value check is needed as there's no limit yet */
		NVSetAttribute(dpy, screen, disp_mask, option, value);
	}
	/* bitmask */
	else if(res->type == 2)
	{
		/* Do nothing as I don't have an option of this type yet */
	}
	/* boolean */
	else if(res->type == 3)
	{
		if(value > 1 || value < 0)
		{
			printf("Error: unsupported value %d for '%s'\n", value, opt->name);
			printf("Supported values for this option are: 0 and 1\n");
			return;
		}
		else
			NVSetAttribute(dpy, screen, disp_mask, option, value);
	}
	/* range */
	else if(res->type == 4)
	{
		int app_controlled=0;
		/* In case of FSAA and Aniso the application can decide what it likes to use.
		/  As most applications don't support FSAA/Aniso there's a setting which allows
		/  the user to choose settings himself. To allow user modifications application
		/  control needs to be disabled else the changes don't do anything.
		/
		/  The code below checks if application control is enabled for FSAA/Aniso and if
		/  it is in case we want to adjust FSAA/Aniso it disables application control.
		*/
		if(option == NV_LOG_ANISO)
		{
			if(NVGetAttribute(dpy, screen, disp_mask, NV_ANISO_APP_CONTROLLED, &app_controlled))
			{
				if(app_controlled)
				{
					NVSetAttribute(dpy, screen, disp_mask, NV_ANISO_APP_CONTROLLED, 0);
				}
			}
		}

		if(value < res->data1 || value > res->data2)
		{
			printf("Error: unsupported value %d for '%s';  ", value, opt->name);
			printf("Supported values lie in the range from %d to %d\n", res->data1, res->data2);
			return;
		}
		else
			NVSetAttribute(dpy, screen, disp_mask, option, value);
	}
	/* bitmask */
	else if(res->type == 5)
	{
		int app_controlled=0;

		/* In case of FSAA/Aniso the application or user can decide which settings to use.
		/  For modifactions done by the user application control needs to be disabled. For
		/  more info read the comment located near the NV_ANISO check above.
		*/
		if(option == NV_FSAA)
		{
			if(NVGetAttribute(dpy, screen, disp_mask, NV_FSAA_APP_CONTROLLED, &app_controlled))
			{
				if(app_controlled)
				{
					NVSetAttribute(dpy, screen, disp_mask, NV_FSAA_APP_CONTROLLED, 0);
				}
			}
		}

		if ((value > 31) || (value < 0) || ((res->data1 & (1<<value)) == 0))
		{
			int i;
			printf("Error: unsupported value %d for '%s'\n", value, opt->name);
			printf("Supported values are:");
			for(i=0; i <= 10; i++)
			{
				if((res->data1 >> i) & 0x1)
				{
					printf(" %d", i);
				}
			}
			printf("\n");
			return;
		}
		else
			NVSetAttribute(dpy, screen, disp_mask, option, value);
	}

	NVGetAttribute(dpy, screen, disp_mask, option, &val);
	printf("Option '%s' set to %d\n", opt->name, val);

	free(res);
}
#endif

/* On various NV4x cards it is possible to enable additional
/  pixel and vertex units. On the commandline the user enters
/  a mask in binary/hexadecimal containing the pipelines to use.
/  This function convert the hex or binary value to a mask of several
/  bits that can be passed to the modding functions.
*/
unsigned char nv40_parse_units_mask(char *mask)
{
	int i, check, value=0;

	/* Check if binary */
	for(i=0, check=1; mask[i] && check; i++)
		check = ((mask[i] == '0') || (mask[i] == '1')) ? 1 : 0;

	if(check)
		value = strtol(mask, (char**)NULL, 2);

	/* Check if hex in case the value wasn't binary */
	for(i=0, check=1; mask[i] && check && !value; i++)
	{
		check = isxdigit((unsigned char)mask[i]);

		/* strtoll supports both ff and 0xff */
		if(!check && i==1)
			check = (mask[i] == 'x') ? 1 : 0;
	}

	/* Only convert if the value is hex; 10 / 11 are considered binary */
	if(check && !value)
		value = strtol(mask, (char**)NULL, 16);

	if((value > 0) && (value < 256))
		return (unsigned char)value;

	return 0;
}


void unload_nvclock()
{
#ifdef HAVE_NVCONTROL
	/* Close the X display */
	if(nvclock.dpy)
		XCloseDisplay(nvclock.dpy);
#endif

	/* Free the config file structure */
	if(nvclock.cfg)
		destroy(&nvclock.cfg);
}


int main(int argc, char *argv[])
{
	int backend, card_number, deviceid, opt;
	float memclk, nvclk;
	short backend_opt, card_opt, debug_opt, fanspeed_opt, force_opt, deviceid_opt, reset_opt, smartdimmer_opt, speeds_opt, temp_opt;
	short assign_opt, info_opt, list_opt, query_opt;
	short punit_opt, vunit_opt;
	char *fanspeed = NULL;
	//char *screen = NULL;
	//char *smartdimmer = NULL;
	char punit, vunit;
#ifdef HAVE_NVCONTROL
	Display *dpy = NULL;
#endif

	assign_opt = 0;
	backend_opt = 0;
	card_opt = 0;
	debug_opt = 0;
	deviceid_opt = 0;
	fanspeed_opt = 0;
	force_opt = 0;
	info_opt = 0;
	list_opt = 0;
	query_opt = 0;
	punit_opt = 0;
	reset_opt = 0;
	speeds_opt = 0;
	smartdimmer_opt = 0;
	temp_opt = 0;
	vunit_opt = 0;

	backend = 0;
	card_number = 0;
	deviceid = 0;
	memclk = 0;
	nvclk = 0;
	punit = 0;
	vunit = 0;
	nvclock.dpy = NULL;

	/* If no options are given. */
	if (argc == 1)
	{
		usage();
		return 0;
	}

	if(!init_nvclock())
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return 0;
	}
#if 0
/* We don't advertise the OpenGL options if we are building without X */
#ifndef HAVE_NVCONTROL
	while ( ( opt = getopt_long (argc, argv, "m:n:b:c:F:P:Q:S:V:fidDrsTh", long_options, NULL)) != -1 )
#else
		while ( ( opt = getopt_long (argc, argv, "m:n:b:a:q:c:F:P:Q:S:V:x:lfidDrsTh", long_options, NULL)) != -1 )
#endif
	{
		switch (opt)
		{
			case 'b':
				backend_opt = 1;
				if(!strcasecmp(optarg, "lowlevel"))
					backend = STATE_LOWLEVEL;
				else if(!strcasecmp(optarg, "coolbits2d"))
					backend = STATE_2D;
				else if(!strcasecmp(optarg, "coolbits3d"))
					backend = STATE_3D;
				else if(!strcasecmp(optarg, "coolbits"))
					backend = STATE_BOTH;
				else
				{
					printf("Invalid backend '%s' use coolbits/coolbits2d/coolbits3d or lowlevel\n", optarg);
					return 0;
				}
				break;
			case 'c':
				card_number = strtol(optarg, (char **)NULL, 10)-1;
				/* If the user only the card number. */
				if(argc == 3)
				{
					printf("Error: You only used the -c option\n");
					return 0;
				}

				/* Check if the card number is valid; Note that internally card_number is 1 smaller than the entered value. */
				if((card_number < 0) || (card_number >= nvclock.num_cards))
				{
					printf("Error: You entered an invalid card number!\nUse the -s option to show all card numbers\n\n");
					return 0;
				}

				card_opt = 1;
				break;

			case 'd':
				/* If the user only entered the -d option */
				if(argc == 2)
				{
					printf("Error: You only used the -d option\n");
					return 0;
				}
				nv_card->debug = 1;
				break;

			case 'D':
				debug_opt = 1;
				break;

			case 'F':
				fanspeed = (char*)STRDUP(optarg, sizeof(optarg));//???
				fanspeed_opt = 1;
				break;
			case 'f':
				/* If the user only entered the -f option */
				if(argc == 2)
				{
					printf("Error: You only used the -f option\n");
					return 0;
				}
				force_opt = 1;
				break;

			case 'i':
				info_opt = 1;
				break;

			case 'm':
				memclk = strtol(optarg, (char **)NULL, 10);

				if(memclk < 0)
				{
					printf("Wrong value for memclk: %f\n", memclk);
					return 1;
				}
				break;

			case 'n':
				nvclk = strtol(optarg, (char **)NULL, 10);

				if(nvclk < 0)
				{
					printf("Wrong value of nvclk: %f\n", nvclk);
					return 1;
				}
				break;

			case 'P':
				punit = nv40_parse_units_mask(optarg);
				if(!punit)
				{
					printf("Wrong value '%s' for pixel unit mask, the value needs to be a binary/hexadecimal number between 0 and 255 (decimal).\n", optarg);
					return 1;
				}
				punit_opt=1;
				break;

			case 'Q':
				deviceid = strtol(optarg, (char **)NULL, 10);
				deviceid_opt = 1;
				break;
			case 'T':
				temp_opt = 1;
				break;
			case 'V':
				vunit = nv40_parse_units_mask(optarg);
				if(!vunit)
				{
					printf("Wrong value '%s' for vertex unit mask, the value needs to be a binary/hexadecimal number between 0 and 255 (decimal).\n", optarg);
					return 1;
				}
				vunit_opt=1;
				break;

			case 'r':
				reset_opt = 1;
				break;

			case 's':
				speeds_opt = 1;
				break;
/* NVControl options */
#ifdef HAVE_NVCONTROL
			case 'a':
				/* If the user only entered the -q option */
				if(argc == 2)
				{
					printf("Error: You only used the -a option\n");
					return 0;
				}
				parse_gl_cmdline_option(optarg, ASSIGN_VALUE);
				assign_opt = 1;
				break;

			case 'l':
				list_opt = 1;
				break;

			case 'q':
				parse_gl_cmdline_option(optarg, QUERY_VALUE);
				query_opt = 1;
				break;

			case 'x':
				/* If the user only entered the -x option */
				if(argc == 2)
				{
					printf("Error: You only used the -x option\n");
					return 0;
				}
				screen = (char*)strdup(optarg);
				break;
#endif
			case 'h':
				usage();
				break;

			default:
				return 0;
		}
	}

	atexit(unload_nvclock);

#ifdef HAVE_NVCONTROL
	if(assign_opt || list_opt || query_opt)
	{
		dpy = XOpenDisplay(screen);
		if(dpy == NULL)
		{
			printf("Can't open screen: %s\n", screen);
			return 0;
		}

		if(!init_nvcontrol(dpy))
		{
			printf("Can't initialize the NV-CONTROL extension which is needed for changing OpenGL settings\n");
			return 0;
		}

		nvclock.dpy = dpy;
	}

	if(list_opt)
	{
		ShowGlAttributes(dpy);
	}

	if(query_opt)
	{
		int i;
		for(i=0; i<query_size; i++)
		{
			GetAttribute(dpy, 0, query_list[i].mask, query_list[i].option);
		}
	}

	if(assign_opt)
	{
		int i;
		for(i=0; i<assign_size; i++)
		{
			SetAttribute(dpy, 0, assign_list[i].mask, assign_list[i].option, assign_list[i].value);
		}
	}
#endif										  /* HAVE_NVCONTROL */

	/* Quit if we don't have anything more to do */
	if(!(backend_opt || debug_opt || deviceid_opt || fanspeed_opt || force_opt || info_opt || punit_opt || reset_opt || smartdimmer_opt || speeds_opt || temp_opt || vunit_opt || memclk || nvclk))
		return 0;

#ifdef HAVE_NVCONTROL
	/* NV3X/NV4X/NV5X can use low-level access and Coolbits for overclocking.
	/  Coolbits is part of the NV-CONTROL extension for X and because of this
	/  needs a X Display. The hack below does this.
	*/
	if(nvclock.card[0].arch & (NV3X | NV4X | NV5X) && (nvclock.dpy == NULL))
		nvclock.dpy = XOpenDisplay("");
#endif  /* HAVE_NVCONTROL */

	/* Check if the user used the -s option, if so show the requested info. */
	/* Detect all cards */
	if(speeds_opt)
	{
		int i;
		for(i=card_number; i< nvclock.num_cards; i++)
		{
			if(!set_card(i))
			{
				char buf[80];
				printf("Error: %s\n", get_error(buf, 80));
				return 0;
			}

			printf("Card: \t\t%s\n", nv_card->card_name);
			printf("Card number: \t%d\n", i+1);

			if(nv_card->caps & COOLBITS_OVERCLOCKING)
			{
				printf("Mode\t\tGPU Clock\tMemory Clock\n");
				nv_card->set_state(STATE_2D);
				printf("Coolbits 2D: \t%0.3f MHz\t%0.3f MHz\n", nv_card->get_gpu_speed(), nv_card->get_memory_speed());
				nv_card->set_state(STATE_3D);
				printf("Coolbits 3D: \t%0.3f MHz\t%0.3f MHz\n", nv_card->get_gpu_speed(), nv_card->get_memory_speed());
				nv_card->set_state(STATE_LOWLEVEL);
				printf("Current: \t%0.3f MHz\t%0.3f MHz\n\n", nv_card->get_gpu_speed(), nv_card->get_memory_speed());
			}
			else
			{
				if(nv_card->gpu == NFORCE)
					printf("Memory clock: \t-\n");
				else
					printf("Memory clock: \t%0.3f MHz\n", nv_card->get_memory_speed());
				printf("GPU clock: \t%0.3f MHz\n\n", nv_card->get_gpu_speed());
			}
			/* Detect only the requested card */
			if(card_opt)
				break;
		}
		return 0;
	}

	/* set the card object to the requested card */
	if(!set_card(card_number))
	{
		char buf[80];
		printf("Error: %s\n", get_error(buf, 80));
		return 0;
	}

	/* Make NVClock work on unsupported cards and access higher speeds as requested by the user */
	if(force_opt && nv_card->gpu == UNKNOWN)
	{
		nvclock.card[card_number].gpu = DESKTOP;
		nv_card->number = -1;  /* Force a re-init of the function pointers */
		set_card(card_number);
	}

	/* Check if the card is supported, if not print a message. */
	if((nvclock.card[card_number].gpu == UNKNOWN) && (force_opt == 0))
	{
		printf("It seems your card isn't officialy supported in NVClock yet.\n");
		printf("The reason can be that your card is too new.\n");
		printf("If you want to try it anyhow [DANGEROUS], use the option -f to force the setting(s).\n");
		printf("NVClock will then assume your card is a 'normal', it might be dangerous on other cards.\n");
		printf("Also please email the author the pci_id of the card for further investigation.\n[Get that value using the -i option].\n\n");
		return 0;
	}

	/* Print debug info for debugging purposes; This is different from the -d switch
	/  which only adds debugging to low-level clock functions.
	*/
	if(debug_opt)
	{
		nv_card->get_debug_info();
		return 0;
	}

	if(deviceid_opt)
	{
		if(!(nv_card->caps & GPU_ID_MODDING))
		{
			printf("Error: Your card doesn't support device id adjustments!\nThis can either be because your GPU is a Riva TNT/TNT2 or if your card is using a AGP->PCI-Express / PCI-Express->AGP bridge chip.\n");
			return 0;
		}
		
		/* We support both specifying the digit and a full device id */
		if((deviceid > 16) && ((deviceid & 0xfffffff0) != (nv_card->device_id & 0xfff0)))
		{
			printf("Error: Invalid value! Specify either a correct digit (<16) or a full device id in decimal.\n");
			return 0;
		}	

		if(!nv_card->set_gpu_pci_id(deviceid & 0xf))
			printf("Error: Something went wrong during pci id adjustment. Most likely you are using a Geforce1/2 card in which case the specified 'digit' needs to be smaller than 4.\n");
		printf("Adjusted the pci id to 0x%x (%s)\n", nv_card->device_id, nv_card->card_name);
	}

	if(fanspeed_opt && force_opt)
	{
		float dutycycle;

		if(!(nv_card->caps & (GPU_FANSPEED_MONITORING | I2C_FANSPEED_MONITORING)))
		{
			printf("Error: Your card doesn't support fanspeed adjustments!\n");
			return 0;
		}

		if(fanspeed[0] == '+' || fanspeed[0] == '-')
		{
			if(nv_card->caps & I2C_FANSPEED_MONITORING)
				dutycycle = nv_card->get_i2c_fanspeed_pwm(nv_card->sensor);
			else
				dutycycle = nv_card->get_fanspeed();

			dutycycle += (float)strtol(fanspeed, (char**)NULL, 10);
			if((dutycycle < 10) || (dutycycle > 100))
			{
				printf("Error: The proposed fanspeed change would result in a fanspeed lower than 10%% or higher than 100%%.\n");
				return 0;
			}
		}
		else if(strcmp(fanspeed, "auto") == 0)
		{
			if(!(nv_card->caps & I2C_AUTOMATIC_FANSPEED_CONTROL))
			{
				printf("Error: This card doesn't support automatic fanspeed adjustments.\n");
				return 0;
			}
		}
		else
		{
			dutycycle = (float)strtol(fanspeed, (char**)NULL, 10);
			if((dutycycle < 10) || (dutycycle > 100))
			{
				printf("Error: Incorrect fanspeed '%.1f', you need to choose a value between 10%% and 100%%.\n", dutycycle);
				return 0;
			}
		}

		/* First process cards with 'advanced' sensor chips */
		if(nv_card->caps & I2C_FANSPEED_MONITORING)
		{
			if(strcmp(fanspeed, "auto") == 0)
			{
				nv_card->set_i2c_fanspeed_mode(nv_card->sensor, 0); /* Put the sensor back in auto mode */
				printf("New fanspeed mode: %s\n", nv_card->get_i2c_fanspeed_mode(nv_card->sensor) ? "manual" : "auto");
			}
			else
			{
				printf("Current fanspeed: %d RPM\n", 	nv_card->get_i2c_fanspeed_rpm(nv_card->sensor));
				printf("PWM duty cycle: %.1f%%\n", 	nv_card->get_i2c_fanspeed_pwm(nv_card->sensor));
				printf("Changing duty cycle from %.1f to %.1f\n", 	nv_card->get_i2c_fanspeed_pwm(nv_card->sensor), dutycycle);
				/* speed is a value between 10% and 100% */
				nv_card->set_i2c_fanspeed_pwm(nv_card->sensor, dutycycle);

				/* It takes a short time for the fanspeed to change */
				IOSleep(10000);
				printf("Fanspeed: %d RPM\n", nv_card->get_i2c_fanspeed_rpm(nv_card->sensor));
				printf("New PWM duty cycle: %.1f\n", nv_card->get_i2c_fanspeed_pwm(nv_card->sensor));
			}
		}
		/* Various standard GeforceFX/6 also have some fanspeed monitoring support */
		else if(nv_card->caps & GPU_FANSPEED_MONITORING)
		{
			printf("Current fanspeed: %.1f%%\n", nv_card->get_fanspeed());
			printf("Changing fanspeed from %.1f%% to %.1f%%\n", nv_card->get_fanspeed(), dutycycle);
			nv_card->set_fanspeed(dutycycle);
			printf("New fanspeed: %.1f%%\n", nv_card->get_fanspeed());
		}
		delete fanspeed;
	}
	else if(fanspeed_opt)
	{
		if(nv_card->caps & (GPU_FANSPEED_MONITORING | I2C_FANSPEED_MONITORING))
		{
			printf("Error!\n");
			printf("While NVClock can adjust the fanspeed of your videocard this features is disabled by default because of safety reasons.!\n");
			printf("If you really know what you are doing you can enable it by adding the -f switch to the nvclock command.\n");
		}
		else
			printf("Error: adjustment of the fanspeed isn't supported on your type of videocard!\n");

		return 0;
	}

	
	if(punit_opt && (nv_card->caps & PIPELINE_MODDING) && force_opt)
	{
		char pmask, pmask_default, vmask, vmask_default;
		char pmask_str[8];
		unsigned char hw_default = 0;
		int punits, total;

		nv_card->get_default_mask(&pmask_default, &vmask_default);
		if(nv_card->get_sw_masked_units(&pmask, &vmask))
		{
			hw_default = ~pmask & pmask_default;
		}
		else if(nv_card->get_hw_masked_units(&pmask, &vmask))
		{
			hw_default = ~pmask & pmask_default;
		}
		else /* No locked units, so nothing left to enable */
		{
			printf("Error: Your card doesn't contain any extra pixel units to enable.\n");
			return 0;
		}

		/* We can't enable more pixel units then available */
		if(punit > pmask_default)
		{
			printf("Error: Illegal mask '%x', can't enable more pixel units than your card contains ('%x')!\n", punit, hw_default);
			return 0;
		}

		punits = nv_card->get_pixel_pipelines(&pmask, &total);
		convert_unit_mask_to_binary(pmask, pmask_default, pmask_str);
		printf("Current pixel unit configuration: %dx%d (%sb)\n", total, punits, pmask_str);

		nv_card->set_pixel_pipelines(punit);
		punits = nv_card->get_pixel_pipelines(&pmask, &total);
		convert_unit_mask_to_binary(pmask, pmask_default, pmask_str);
		printf("New pixel unit configuration: %dx%d (%sb)\n", total, punits, pmask_str);
	}
	else if(punit_opt)
	{
		if(nv_card->caps & PIPELINE_MODDING)
		{
			printf("Error!\n");
			printf("While pipeline modding is supported on your card it is disabled by default because of safety reasons.\n");
			printf("Pipeline modding can't be used while the Nvidia driver is active (exit X and unload the kernel module).\n");
			printf("If you don't follow these instructions there's a big chance that your computer will freeze.\n");
			printf("When you are ready and know what you are doing enable this option using the -f switch.\n");
		}
		else
			printf("Error: your card doesn't support pipeline modding\n");

		return 0;
	}

	if(vunit_opt && (nv_card->caps & PIPELINE_MODDING) && force_opt)
	{
		char pmask, pmask_default, vmask, vmask_default;
		char vmask_str[8];
		unsigned char hw_default = 0;
		int vunits;

		nv_card->get_default_mask(&pmask_default, &vmask_default);
		if(nv_card->get_sw_masked_units(&pmask, &vmask))
		{
			hw_default = ~vmask & vmask_default;
		}
		else if(nv_card->get_hw_masked_units(&pmask, &vmask))
		{
			hw_default = ~vmask & vmask_default;
		}
		else /* No locked units, so nothing left to enable */
		{
			printf("Error: Your card doesn't contain any extra vertex units to enable.\n");
			return 0;
		}

		/* We can't enable more vertex units then available */
		if(vunit > vmask_default)
		{
			printf("Error: Illegal mask '%x', can't enable more vertex units than your card contains ('%x')!\n", punit, hw_default);
			return 0;
		}

		vunits = nv_card->get_vertex_pipelines(&vmask);
		convert_unit_mask_to_binary(vmask, vmask_default, vmask_str);
		printf("Current pixel unit configuration: %dx1 (%sb)\n", vunits, vmask_str);

		nv_card->set_vertex_pipelines(vunit);
		vunits = nv_card->get_vertex_pipelines(&vmask);
		convert_unit_mask_to_binary(vmask, vmask_default, vmask_str);
		printf("New vertex unit configuration: %dx1 (%sb)\n", vunits, vmask_str);
	}
	else if(vunit_opt)
	{
		if(nv_card->caps & PIPELINE_MODDING)
		{
			printf("Error!\n");
			printf("While pipeline modding is supported on your card it is disabled by default because of safety reasons.\n");
			printf("Pipeline modding can't be used while the Nvidia driver is active (exit X and unload the kernel module).\n");
			printf("If you don't follow these instructions there's a big chance that your computer will freeze.\n");
			printf("When you are ready and know what you are doing enable this option using the -f switch.\n");
		}
		else
			printf("Error: your card doesn't support pipeline modding.\n");

		return 0;
	}

	if(temp_opt)
	{
		if(nv_card->caps & (GPU_TEMP_MONITORING))
		{
			printf("%s\n", nv_card->card_name);
			printf("=> GPU temperature: %dC\n", nv_card->get_gpu_temp(nv_card->sensor));
			if(nv_card->caps & (BOARD_TEMP_MONITORING))
				printf("=> Board temperature: %dC\n", nv_card->get_board_temp(nv_card->sensor));

#ifdef HAVE_NVCONTROL
			/* Some NV-CONTROL debugging code useful for calibration */
			if(nv_card->debug && nvclock.dpy)
			{
				int gpu_temp, board_temp;
				if(NVGetAttribute(nvclock.dpy, 0, 0, NV_GPU_TEMPERATURE, &gpu_temp))
					printf("=> GPU temperature according to NV-CONTROL: %d\n", gpu_temp);
				if(NVGetAttribute(nvclock.dpy, 0, 0, NV_AMBIENT_TEMPERATURE, &board_temp))
					printf("=> Board temperature according to NV-CONTROL: %d\n", board_temp);
			}
#endif
		}
		else
			printf("Error: temperature monitoring isn't supported on your videocard.\n");
		return 0;
	}

	/* Get the card information for the currently selected card */
	if(info_opt)
	{
		char arch[10];

		/* In case of NV3x/NV4x/NV5x boards make sure we use the low-level path */
		if((nv_card->caps & COOLBITS_OVERCLOCKING)  && (nv_card->state != STATE_LOWLEVEL))
			nv_card->set_state(STATE_LOWLEVEL);

		printf("-- General info --\n");
		printf("Card: \t\t%s\n",  nv_card->card_name);
		
		convert_gpu_architecture(nv_card->get_gpu_architecture(), arch);
		printf("Architecture: \t%s %X\n", arch, nv_card->get_gpu_revision());
		printf("PCI id: \t0x%x\n", nv_card->device_id);
		printf("GPU clock: \t%0.3f MHz\n", nv_card->get_gpu_speed());
		printf("Bustype: \t%s\n\n", nv_card->get_bus_type());

		/* Pipeline info is supported on NV4X cards */
		if(nv_card->arch & NV4X)
		{
			char pmask, pmask_default, vmask, vmask_default;
			char pmask_str[8], vmask_str[8];
			int pixel_pipes, vertex_pipes, total;

			nv_card->get_default_mask(&pmask_default, &vmask_default);

			printf("-- Pipeline info --\n");
			pixel_pipes = nv_card->get_pixel_pipelines(&pmask, &total);
			convert_unit_mask_to_binary(pmask, pmask_default, pmask_str);
			printf("Pixel units: %dx%d (%sb)\n", pixel_pipes, total, pmask_str);
			vertex_pipes = nv_card->get_vertex_pipelines(&vmask);
			convert_unit_mask_to_binary(vmask, vmask_default, vmask_str);
			printf("Vertex units: %dx1 (%sb)\n", vertex_pipes, vmask_str);

			if(nv_card->get_hw_masked_units(&pmask, &vmask))
			{
				convert_unit_mask_to_binary(pmask, pmask_default, pmask_str);
				convert_unit_mask_to_binary(vmask, vmask_default, vmask_str);
				printf("HW masked units: pixel %sb vertex %sb\n", pmask_str, vmask_str);
			}
			else
				printf("HW masked units: None\n");

			if(nv_card->get_sw_masked_units(&pmask, &vmask))
			{
				convert_unit_mask_to_binary(pmask, pmask_default, pmask_str);
				convert_unit_mask_to_binary(vmask, vmask_default, vmask_str);
				printf("SW masked units: pixel %sb vertex %sb\n\n", pmask_str, vmask_str);
			}
			else
				printf("SW masked units: None\n\n");
		}

		if(nv_card->arch & NV5X)
		{
			char smask, smask_default, rmask, rmask_default;
			char smask_str[8], rmask_str[8];
			int stream_units, rop_units;

			printf("-- Shader info --\n");
			printf("Clock: %0.3f MHz\n", nv_card->get_shader_speed());
			stream_units = nv_card->get_stream_units(&smask, &smask_default);
			convert_unit_mask_to_binary(smask, smask_default, smask_str);
			printf("Stream units: %d (%sb)\n", stream_units, smask_str);
			rop_units= nv_card->get_rop_units(&rmask, &rmask_default);
			convert_unit_mask_to_binary(rmask, rmask_default, rmask_str);
			printf("ROP units: %d (%sb)\n", rop_units, rmask_str);
		}

		printf("-- Memory info --\n");
		printf("Amount: \t%d MB\n", nv_card->get_memory_size());
		printf("Type: \t\t%d bit %s\n", nv_card->get_memory_width(), nv_card->get_memory_type());

		/* nForce cards don't have local video memory */
		if(nv_card->gpu == NFORCE)
			printf("Clock: \t\t-\n\n");
		else
			printf("Clock: \t\t%0.3f MHz\n\n", nv_card->get_memory_speed());


		if(strcmp(nv_card->get_bus_type(), "AGP") == 0)
		{
			printf("-- AGP info --\n");
			printf("Status: \t%s\n", nv_card->get_agp_status());
			printf("Rate: \t\t%dX\n", nv_card->get_bus_rate());
			printf("AGP rates: \t%s\n", nv_card->get_agp_supported_rates());
			printf("Fast Writes: \t%s\n", nv_card->get_agp_fw_status());
			printf("SBA: \t\t%s\n\n", nv_card->get_agp_sba_status());
		}

		if(strcmp(nv_card->get_bus_type(), "PCI-Express") == 0)
		{
			printf("-- PCI-Express info --\n");
			printf("Current Rate: \t%dX\n", nv_card->get_bus_rate());
			printf("Maximum rate: \t%dX\n\n", nv_card->get_pcie_max_bus_rate());
		}

		if(nv_card->caps & SMARTDIMMER)
		{
			printf("-- Smartdimmer info --\n");
			printf("Backlight level: %d%%\n\n", nv_card->get_smartdimmer());
		}

		/* On some Geforce 6600(GT) cards, we can adjust the fans while we can't access the sensor yet */
		if(nv_card->caps & (BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING | GPU_FANSPEED_MONITORING))
		{
			printf("-- Sensor info --\n");
			printf("Sensor: %s\n", nv_card->sensor_name);

			if(nv_card->caps & BOARD_TEMP_MONITORING)
				printf("Board temperature: %dC\n", nv_card->get_board_temp(nv_card->sensor));

			if(nv_card->caps & GPU_TEMP_MONITORING)
				printf("GPU temperature: %dC\n", nv_card->get_gpu_temp(nv_card->sensor));

			/* Cards equipped with sensors like the Fintek F75375 use this chip for fanspeed stuff */
			if(nv_card->caps & I2C_FANSPEED_MONITORING)
			{
				printf("Fanspeed: %d RPM\n", nv_card->get_i2c_fanspeed_rpm(nv_card->sensor));
				if(nv_card->caps & I2C_AUTOMATIC_FANSPEED_CONTROL)
					printf("Fanspeed mode: %s\n", nv_card->get_i2c_fanspeed_mode(nv_card->sensor) ? "manual" : "auto");
				printf("PWM duty cycle: %.1f%%\n", nv_card->get_i2c_fanspeed_pwm(nv_card->sensor));
			}
			/* Nvidia reference boards allow fanspeed adjustments/monitoring using a special register */
			else if(nv_card->caps & GPU_FANSPEED_MONITORING)
				printf("Fanspeed: %.1f%%\n", nv_card->get_fanspeed());

			printf("\n");
		}

		if(nv_card->bios)
		{
			int i;
			printf("-- VideoBios information --\n");
			printf("Version: %s\n", nv_card->bios->version);
			printf("Signon message: %s\n", nv_card->bios->signon_msg);

			for(i=0; i< nv_card->bios->perf_entries; i++)
			{
				printf("Performance level %d: gpu %d", i, nv_card->bios->perf_lst[i].nvclk);
				if(nv_card->bios->perf_lst[i].delta)
					printf("(+%d)", nv_card->bios->perf_lst[i].delta);
				if(nv_card->bios->perf_lst[i].shaderclk)
					printf("MHz/shader %d", nv_card->bios->perf_lst[i].shaderclk);
				printf("MHz/memory %dMHz", nv_card->bios->perf_lst[i].memclk);
				if(nv_card->bios->volt_entries)
					printf("/%.2fV", nv_card->bios->perf_lst[i].voltage);
				if(nv_card->bios->perf_lst[i].fanspeed)
					printf("/%d%%", nv_card->bios->perf_lst[i].fanspeed);
				printf("\n");
			}

			if(nv_card->bios->volt_entries)
				printf("VID mask: %x\n", nv_card->bios->volt_mask);

			for(i=0; i< nv_card->bios->volt_entries; i++)
			{
				/* For now assume the first memory entry is the right one; should be fixed as some bioses contain various different entries */
				/* Note that voltage entries in general don't correspond to performance levels!! */
				printf("Voltage level %d: %.2fV, VID: %x\n", i, nv_card->bios->volt_lst[i].voltage, nv_card->bios->volt_lst[i].VID);
			}
			printf("\n");
		}
		return 0;
	}

	if(backend_opt)
	{
		if(backend == STATE_LOWLEVEL)
			nv_card->set_state(STATE_LOWLEVEL);
		else if((nv_card->caps & COOLBITS_OVERCLOCKING) && nvclock.dpy)
			nv_card->set_state(backend);
		else if(!nvclock.dpy) /* If not lowlevel then we want to use Coolbits */
		{
			printf("Error:\n");
			printf("Can't switch to the Coolbits backend because X isn't loaded\n");
			return 0;
		}
		else if(!(nv_card->arch & (NV3X | NV4X | NV5X )))
		{
			printf("Error:\n");
			printf("Can't switch to the Coolbits backend because it requires a GeforceFX/6/7/8 card.\n");
			return 0;
		}
		else if(nv_card->gpu == MOBILE)
		{
			printf("Error:\n");
			printf("Can't switch to the Coolbits backend because either it isn't supported on your laptop driver yet (requires 169.x) or you don't have it enabled in your xorg.conf.\n");
			return 0;
		}
		else
		{
			printf("Error:\n");
			printf("Can't switch to the Coolbits backend because the NV-CONTROL extension wans't found.\nThis can happen when you aren't using Nvidia driver 1.0-7676 or newer or when the option isn't enabled in the X config file.\n");
			return 0;
		}
	}

	/* When the backend option isn't specified we default to coolbits when it is available */
	if(!backend_opt && nv_card->caps & COOLBITS_OVERCLOCKING)
		nv_card->set_state(STATE_BOTH);

	if((nv_card->state == STATE_LOWLEVEL) && (nv_card->arch & NV5X))
	{
		printf("Error: NVClock doesn't offer lowlevel overclocking on NV50/G8x/G9x/GT200 hardware (yet).\nIf you want to overclock your card using the Nvidia drivers instead add the line:\n Option \"Coolbits\" \"1\" to the screen or device section in your xorg.conf and then try NVClock again.\n");
		return 0;
	}

	if(reset_opt)
	{
		if(nv_card->gpu == MOBILE && !force_opt)
		{
			printf("Error: By default overclocking is disabled on laptops. If you know what you are doing enable it using the -f option.\n");
			return 0;
		}

		nv_card->reset_gpu_speed();
		nv_card->reset_memory_speed();
		printf("Your %s has been restored to its original clocks\n", nv_card->card_name);
		printf("Memory clock: \t%0.3f MHz\n", nv_card->get_memory_speed());
		printf("GPU clock: \t%0.3f MHz\n\n", nv_card->get_gpu_speed());
		return 0;
	}

	/* Check if the gpu speed is higher than NVClock's max speed (+25%), if not print a message. */
	if( (nvclk >= nv_card->nvclk_max) && force_opt == 0)
	{
		printf("Warning!\n");
		printf("You entered a core speed of %.3f MHz and NVClock believes %d.000 MHz is the maximum!\n", nvclk, nv_card->nvclk_max);
		printf("This error appears when the entered speed is 25%% higher than the default speed.\n");
		printf("If you really want to use this speed, use the option -f to force it.\n\n");
		return 0;
	}

	/* Check if the memory speed is higher than NVClock's max speed (+25%), if not print a message. */
	if( (memclk >= nv_card->memclk_max) && force_opt == 0)
	{
		printf("Warning!\n");
		printf("You entered a memory speed of %.3f MHz and NVClock believes %d.000 MHz is the maximum!\n", memclk, nv_card->memclk_max);
		printf("This error appears when the entered speed is 25%% higher than the default speed.\n");
		printf("If you really want to use this speed, use the option -f to force it.\n\n");
		return 0;
	}

	if(memclk != 0)
	{
		/* Check if memory overclocking is supported; this isn't the case for the nforce where there's no real video memory */
		if((nv_card->caps & MEM_OVERCLOCKING) || (nv_card->gpu == MOBILE && force_opt))
		{
			printf("Requested memory clock: \t%0.3f MHz\n", memclk);
			nv_card->set_memory_speed(memclk);
		}
		else if(nv_card->gpu == MOBILE)
		{
			printf("Error: Memory overclocking is disabled on laptops. If you know what you are doing enable it using the -f option.\n");
			return 0;
		}
		else if(nv_card->gpu == NFORCE)
		{
			printf("Error: Memory overclocking isn't supported on NForce cards because system memory is used for Video.\n");
			return 0;
		}
		else
		{
			printf("Error: Memory overclocking isn't supported on your videocard (yet).\n");
			return 0;
		}
	}

	if(nvclk != 0)
	{
		if((nv_card->caps & GPU_OVERCLOCKING) || (nv_card->gpu == MOBILE && force_opt))
		{
			printf("Requested core clock: \t\t%0.3f MHz\n", nvclk);
			nv_card->set_gpu_speed(nvclk);
		}
		else if(nv_card->gpu == MOBILE)
		{
			printf("Error: GPU overclocking is disabled on laptops. If you know what you are doing enable it using the -f option.\n");
			return 0;
		}
		else
		{
			printf("Error: GPU overclocking isn't supported on your videocard (yet).\n");
			return 0;
		}
	}

	/* Only show the speeds when the user is overclocking */
	if(memclk || nvclk)
	{
		if((nv_card->caps & COOLBITS_OVERCLOCKING) && (nv_card->state != STATE_LOWLEVEL))
		{
			if(nv_card->state == STATE_2D)
				printf("\nAdjusted Coolbits 2D clocks on a %s\n", nv_card->card_name);
			else if(nv_card->state == STATE_3D)
				printf("\nAdjusted Coolbits 3D clocks on a %s\n", nv_card->card_name);
			else
				printf("\nAdjusted Coolbits 2D/3D clocks on a %s\n", nv_card->card_name);
		}
		else
			printf("\nAdjusted low-level clocks on a %s\n", nv_card->card_name);

		printf("Memory clock: \t%0.3f MHz\n", nv_card->get_memory_speed());
		printf("GPU clock: \t%0.3f MHz\n\n", nv_card->get_gpu_speed());
	}
#endif
	return 0;
}
