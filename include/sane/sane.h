/*********************************************************************************
 *    sane - Scanner Access Now Easy.                                            *
 *    Copyright (C) 1997-1999 David Mosberger-Tang and Andreas Beck              *
 *    This file is part of the SANE package.                                     *
 *                                                                               *
 *    This file is in the public domain.  You may use and modify it as           *
 *    you see fit, as long as this copyright message is included and             *
 *    that there is an indication as to what modifications have been             *
 *    made (if any).                                                             *
 *                                                                               *
 *    SANE is distributed in the hope that it will be useful, but WITHOUT        *
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      *
 *    FITNESS FOR A PARTICULAR PURPOSE.                                          *
 *                                                                               *
 *    This file declares SANE application interface.  See the SANE               *
 *    standard for a detailed explanation of the interface.                      *
 *********************************************************************************/


/**
 * @file sane.h
 * @author yusq
 * @date 2020年 03月 13日 星期五 17:28:03 CST
 * @brief sane.h API规范文档
 *
 * @details sane API接口文档用于扫描软件的研发
 */

#ifndef sane_h
#define sane_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SANE types and defines
 */

#define SANE_CURRENT_MAJOR	1
#define SANE_CURRENT_MINOR	0

#define SANE_VERSION_CODE(major, minor, build)	\
  (  (((SANE_Word) (major) &   0xff) << 24)	\
   | (((SANE_Word) (minor) &   0xff) << 16)	\
   | (((SANE_Word) (build) & 0xffff) <<  0))

#define SANE_VERSION_MAJOR(code)	((((SANE_Word)(code)) >> 24) &   0xff)
#define SANE_VERSION_MINOR(code)	((((SANE_Word)(code)) >> 16) &   0xff)
#define SANE_VERSION_BUILD(code)	((((SANE_Word)(code)) >>  0) & 0xffff)


#define SANE_FALSE	0
#define SANE_TRUE	1

/**
 * SANE标准仅基于SANE特定的两种基本类型：SANE字节和字。
 * typedef some-scalar-type SANE_Byte; 
 * typedef some-scalar-type SANE_Word;
 * SANE_Byte必须与某种能够容纳0到255范围内的值的标量C类型相对应 。SANE_Word必须能够容纳以下任何一项：
 *      真值SANE_FALSE和SANE_TRUE
 *      范围为-2^31 ... 2^31 -1的有 符号整数
 *      定点值范围为-32768 ... 32767.9999，分辨率为1/65536
 *      32位（用于位集）
 * 请注意，SANE标准未定义C类型的 SANE_Byte和SANE_Word映射到。例如，在某些平台上，后者可能映射到long int，
 * 而在其他平台上，它可能映射到int。因此，便携式SANE前端或后端必须不依赖于特定的映射。
 **/


/**
 * 布尔类型
 * SANE_Bool仅仅是一个别名SANE_Word。因此，始终使用后一种代替前一种是合法的。
 * 建议每当给定变量或形式参数对布尔对象具有固定解释时，均应使用 SANE_Bool。
 */
typedef int  SANE_Word;
typedef SANE_Word  SANE_Bool;

/** 
 * 整数类型
 * SANE_Int用于变量，该变量可以采用-2^32至2^31 -1之间的整数值 
 * 每当给定变量或形式参数具有固定解释作为整数对象时，都使用 SANE_Int。
 */
typedef SANE_Word  SANE_Int;

/**
 * 字符类型
 * 类型SANE_Char表示单个文本字符或符号 
 * 目前，此类型直接映射到基础C char类型（通常为一个字节）。此类字符的编码当前固定为ISO LATIN-1。
 * 该标准的未来版本可能将此类型映射到更大的类型，并允许多字节编码支持国际化。
 * 结果，应注意避免sizeof（SANE_Char）== sizeof（char）的假设 。
 */
typedef unsigned char  SANE_Byte;
typedef char SANE_Char;

/**
 * 字符串类型
 * 类型SANE_String将文本字符串表示为C个char值的序列。序列的结尾由'\ 0'（NUL ）字符指示 。 
 * 类型SANE_String_Const由SANE提供，用于声明内容不可更改的字符串。
 */
typedef SANE_Char *SANE_String;
typedef const SANE_Char *SANE_String_Const;

/** 
 * 扫描仪手柄类型
 * 通过称为SANE_Handle的不透明类型可以访问扫描仪 。
 * 尽管此类型被声明为void指针，但应用程序不得尝试解释SANE_Handle的值。
 * 特别是，SANE不需要此类型的值是合法指针值。
 */
typedef void *SANE_Handle;


/**
 * 定点类型
 * SANE_Fixed仅仅是一个别名SANE_Word
 * 建议每当给定变量或形式参数具有固定解释作为定点对象时，都使用 SANE_Fixed。
 */
typedef SANE_Word SANE_Fixed;

/** 
 * 定点类型
 * 用于变量，定点值可以在-32768到32767.9999之间，分辨率为1/65535
 * 宏SANE_FIXED_SCALE_SHIFT给出固定二进制点的位置。该标准将该值定义为16，从而产生1/65536的分辨率。
 */
#define SANE_FIXED_SCALE_SHIFT	16

/** 
 * 并没有要求在以下两个表达式保持为真（即使值w ^和d在范围内）：
 * SANE_UNFIX（SANE_FIX（d））== d
 * SANE_FIX（SANE_UNFIX（w））== w
 * 换句话说，固定值和双精度值之间的转换可能是有损的。因此，建议避免在两种表示形式之间重复转换。
 */

/* 返回小于双精度值d的最大SANE定点值。不执行范围检查。如果d的值超出范围，则结果不确定。 */
#define SANE_FIX(v)	((SANE_Word) ((v) * (1 << SANE_FIXED_SCALE_SHIFT)))
/* 返回与定点值w对应的最接近的双精度机器编号 。 */
#define SANE_UNFIX(v)	((double)(v) / (1 << SANE_FIXED_SCALE_SHIFT))

/* 状态类型
 × 大多数SANE操作都返回SANE_Status类型的值 指示操作是否完成。
 * 如果操作成功完成，则返回SANE_STATUS_GOOD。如果发生错误，将返回一个指示问题性质的值。
 * 建议使用函数 sane_strstatus（）将状态代码转换为清晰的字符串。
 */
typedef enum
{
  SANE_STATUS_GOOD = 0,	/**< everything A-OK, 操作成功完成 */
  SANE_STATUS_UNSUPPORTED,	/**< operation is not supported, 不支持该操作 */
  SANE_STATUS_CANCELLED,	/**< operation was cancelled, 操作被取消 */
  SANE_STATUS_DEVICE_BUSY,	/**< device is busy; try again later, 设备正忙--请稍后重试 */
  SANE_STATUS_INVAL,		/**< data is invalid (includes no dev at open), 数据或参数无效 */
  SANE_STATUS_EOF,		/**< no more data available (end-of-file), 没有更多可用数据（文件结束） */
  SANE_STATUS_JAMMED,		/**< document feeder jammed, 文档进纸器卡纸 */
  SANE_STATUS_NO_DOCS,	/* document feeder out of documents, 文档进纸器中的文档不足 */
  SANE_STATUS_COVER_OPEN,	/**< scanner cover is open, 扫描仪盖板已打开 */
  SANE_STATUS_IO_ERROR,	/**< error during device I/O, 设备I / O期间出错 */
  SANE_STATUS_NO_MEM,		/**< out of memory, 没有内存 */
  SANE_STATUS_ACCESS_DENIED	/**< access to resource has been denied, 对资源的访问已被拒绝 */
}
SANE_Status;

/**
 * following are for later sane version, older frontends wont support 
 */
#if 0
#define SANE_STATUS_WARMING_UP 12 /**< lamp not ready, please retry */
#define SANE_STATUS_HW_LOCKED  13 /**< scanner mechanism locked for transport */
#endif

typedef enum
  {
    SANE_TYPE_BOOL = 0,
    SANE_TYPE_INT,
    SANE_TYPE_FIXED,
    SANE_TYPE_STRING,
    SANE_TYPE_BUTTON,
    SANE_TYPE_GROUP
  }
SANE_Value_Type;

typedef enum
  {
    SANE_UNIT_NONE = 0,		/**< the value is unit-less (e.g., # of scans) */
    SANE_UNIT_PIXEL,		/**< value is number of pixels */
    SANE_UNIT_BIT,		/**< value is number of bits */
    SANE_UNIT_MM,		/**< value is millimeters */
    SANE_UNIT_DPI,		/**< value is resolution in dots/inch */
    SANE_UNIT_PERCENT,		/**< value is a percentage */
    SANE_UNIT_MICROSECOND	/* value is micro seconds */
  }
SANE_Unit;

/**
 * 设备描述符类型
 * 每个SANE设备都由SANE_Device类型的结构表示
 * 该结构在成员名称中提供了扫描仪的唯一 名称。设备结构中的其余成员在设备上提供了与唯一名称相对应的其他信息。
 * 在调用sane_open（）时应传递此唯一名称。此名称的格式完全取决于后端。
 * 唯一的限制是该名称在后端支持的所有设备中都是唯一的，并且该名称是合法的SANE文本字符串。
 * 为了简化唯一名称的表示，其长度不应过多。这是建议的是后端保持低于长度为32个字符的唯一名称。
 * 但是，应用程序 必须能够处理任意长度的唯一名称。
 * 供应商字符串Noname可以用于没有物理供应商关联的虚拟设备。
 * 而且，由于这些字符串是特定于供应商的，因此没有预定义的型号名称字符串，因此完全在各个后端的控制之下。
 */
typedef struct
{
  SANE_String_Const name;	/**< unique device name, 有关卖方（制造商名字） */
  SANE_String_Const vendor;	/**< device vendor string, 制造商 */
  SANE_String_Const model;	/**< device model name, 型号 */
  SANE_String_Const type;	/**< device type (e.g., "flatbed scanner"), 设备类型 */
}
SANE_Device;

#define SANE_CAP_SOFT_SELECT		(1 << 0)
#define SANE_CAP_HARD_SELECT		(1 << 1)
#define SANE_CAP_SOFT_DETECT		(1 << 2)
#define SANE_CAP_EMULATED		(1 << 3)
#define SANE_CAP_AUTOMATIC		(1 << 4)
#define SANE_CAP_INACTIVE		(1 << 5)
#define SANE_CAP_ADVANCED		(1 << 6)

#define SANE_OPTION_IS_ACTIVE(cap)	(((cap) & SANE_CAP_INACTIVE) == 0)
#define SANE_OPTION_IS_SETTABLE(cap)	(((cap) & SANE_CAP_SOFT_SELECT) != 0)

#define SANE_INFO_INEXACT		(1 << 0)
#define SANE_INFO_RELOAD_OPTIONS	(1 << 1)
#define SANE_INFO_RELOAD_PARAMS		(1 << 2)

typedef enum
  {
    SANE_CONSTRAINT_NONE = 0,
    SANE_CONSTRAINT_RANGE,
    SANE_CONSTRAINT_WORD_LIST,
    SANE_CONSTRAINT_STRING_LIST
  }
SANE_Constraint_Type;

typedef struct
  {
    SANE_Word min;		/**< minimum (element) value */
    SANE_Word max;		/**< maximum (element) value */
    SANE_Word quant;	/**< quantization value (0 if none) */
  }
SANE_Range;


/**
 * 选项描述符类型
 * 选项描述符是SANE标准中最复杂，功能最强大的类型。
 * 选项用于控制设备操作的几乎所有方面。
 * SANE API的强大功能主要是因为大多数设备控件均由其各自的选项描述符完全描述。
 * 因此，前端可以抽象地控制扫描仪，而无需了解任何给定选项的目的是什么。
 * 相反，扫描仪可以描述其控件，而无需了解前端的操作方式。
 */
typedef struct
  {
    /**
     * 成员名称
     * 是唯一标识该选项的字符串。该名称对于给定的设备必须是唯一的（即，跨不同后端或设备的选项名称不必是唯一的）。
     * 选项名必须由小写字母ASCII（一 - Ž），数字（0 - 9）或连字符（- ）只。第一个字符必须是小写的ASCII字符（即，不是数字或破折号）。
     */
    SANE_String_Const name;	/**< name of this option (command-line name), 选项名称,  */
    
    /**
     * 选项标题
     * 成员标题是单行字符串，前端可以将其用作标题字符串。通常应该是根据选项功能选择的短字符串（一个或两个单词）。
     */
    SANE_String_Const title;	/**< title of this option (single-line) */

    /** 
     * 选项说明
     * 成员desc是一个（可能非常长）的字符串，可以用作描述该选项的帮助文本。
     * 前端负责将字符串分成可管理长度的行。此字符串中的换行符应解释为段落分隔符。
     */
    SANE_String_Const desc;	/**< description of this option (multi-line) */

    /** 
     * 选项值类型
     * 成员类型指定选项值的类型。
     *   符号 	              码 	描述
     *   SANE_TYPE_BOOL       0 	选项值的类型为 SANE_Bool。
     *   SANE_TYPE_INT        1 	选项值的类型为 SANE_Int。
     *   SANE_TYPE_FIXED      2 	选项值的类型为 SANE_Fixed。
     *   SANE_TYPE_STRING     3 	选项值的类型为 SANE_String。
     *   SANE_TYPE_BUTTON     4 	此类型的选项没有价值。而是，设置此类型的选项会产生特定于选项的副作用。
     *                              例如，后端可以使用按钮类型的选项来提供选择默认值的方法，或者告诉自动文档进纸器前进到下一页纸。
     *   SANE_TYPE_GROUP      5 	此类型的选项没有价值。此类型用于对逻辑相关选项进行分组。
     *                              一个组选项在遇到另一个组选项之前一直有效（或者，如果没有其他组选项，则一直到选项列表的末尾）。
     *                              对于组选项，在选项描述符中仅成员标题和 类型有效。
     */
    SANE_Value_Type type;	/**< how are values interpreted? */

    /**
     * 选项值单位
     * 成员单位指定选项值的物理单位是什么。
     * 指定的单位是SANE后端期望的单位。这些单元如何呈现给用户完全取决于前端。
     * 例如，SANE以毫米表示所有长度。
     * 常期望前端提供适当的转换例程，以便用户可以用惯常单位（例如，英寸或厘米）表示数量。
     *
     * 符号 	                码 	描述
     * SANE_UNIT_NONE  	        0 	值是无单位的（例如，页数）。
     * SANE_UNIT_PIXEL 	        1 	值以像素数为单位。
     * SANE_UNIT_BIT 	        2 	值以位数为单位。
     * SANE_UNIT_MM 	        3 	值以毫米为单位。
     * SANE_UNIT_DPI 	        4 	值是以点/英寸为单位的分辨率。
     * SANE_UNIT_PERCENT 	    5 	值是一个百分比。
     * SANE_UNIT_MICROSECOND 	6 	值是以秒为单位的时间。
     */
    SANE_Unit unit;		/**< what is the (physical) unit? */

    /**
     * 选项值大小
     * 成员大小指定选项值的大小（以字节为单位）。
     * 根据选项值的类型，此成员的解释稍有不同：
     * 
     * SANE_TYPE_STRING：
     * 大小是字符串的最大大小。为了计算字符串大小，将终止NUL字符视为字符串的一部分。请注意，终止NUL字符必须始终出现在字符串选项值中。
     *
     * SANE_TYPE_INT，SANE_TYPE_FIXED：
     * 该大小必须是SANE_Word大小的正整数倍 。选项值是长度的向量
     *
     * size / sizeof（SANE_Word）。
     *
     * SANE_TYPE_BOOL：
     *
     * 大小必须设置为 sizeof（SANE_Word）。
     *
     * SANE_TYPE_BUTTON，SANE_TYPE_GROUP：
     *
     * 选项大小被忽略。
     */
    SANE_Int size;

    /**
     * 选项功能
     * 成员上限描述了该选项具备的功能。这是一个位集，形成为表5中所述功能的包含性逻辑或。
     * SANE API为宏提供了以下功能，以测试给定功能位集的某些功能：
     *
     * SANE_OPTION_IS_ACTIVE （上限）：
     * 当且仅当具有功能设置上限的选项当前处于活动状态时，此宏才返回SANE_TRUE。
     * 
     * SANE_OPTION_IS_SETTABLE （上限）：
     * 当且仅当具有功能设置上限的选项是可软件设置的，此宏才返回SANE_TRUE。
     *
     *     符号 	        码 	描述
     *
     *SANE_CAP_SOFT_SELECT	1 	可以通过调用sane_control_option（）来设置选项值。
     *
     *SANE_CAP_HARD_SELECT	2 	可以通过用户干预（例如，通过拨动开关）来设置选项值。用户界面应提示用户执行适当的操作以设置此类选项。
     *                          此功能与SANE_CAP_SOFT_SELECT互斥（可以设置一个，但不能同时设置）。
     *
     *SANE_CAP_SOFT_DETECT	4 	选项值可以通过软件检测。如果 设置了SANE_CAP_SOFT_SELECT，则必须 设置此功能。
     *                          如果设置了SANE_CAP_HARD_SELECT，则可能会或可能不会设置此功能。如果设置了此功能，
     *                          但是 SANE_CAP_SOFT_SELECT和SANE_CAP_HARD_SELECT 都没有，则无法控制该选项。也就是说，该选项仅提供当前值的读取。
     *
     *SANE_CAP_EMULATED	    8 	如果设置了此功能，则表明该设备不直接支持该选项，而是在后端进行仿真。
     *                          复杂的前端可以选择使用自己的（可能更好）的仿真来代替仿真选项。
     *
     *SANE_CAP_AUTOMATIC	16 	如果设置，此功能表示后端（或设备）能够自动选择合理的选项值。
     *                          对于此类选项，可以通过 使用动作值SANE_ACTION_SET_AUTO调用sane_control_option（）来选择自动操作。
     *
     *SANE_CAP_INACTIVE	    32 	如果设置了此功能，则表明该选项当前未激活（例如，因为仅当另一个选项设置为其他值时才有意义）。
     *
     *SANE_CAP_ADVANCED	    64 	如果设置了此功能，则表明该选项应被视为“高级用户选项”。
     *                          前端通常以比常规选项不那么明显的方式显示此类选项（例如，
     *                          命令行界面可能会在最后列出此类选项或以图形方式界面可能会使它们在单独的``高级设置''对话框中可用）。
     */
    SANE_Int cap;		/**< capabilities */


    /**
     * 选项值限制
     * 约束一个选项可以采用的值通常很有用。例如，前端可以使用约束来确定如何表示给定的选项。
     * 成员constraint_type指示该选项有效的约束条件。该选项允许的约束值由成员约束的并集成员之一描述。
     * SANE_Constraint_Type类型的可能值
     *
     * 符号 	码 	描述
     *
     * SANE_CONSTRAINT_NONE	0 	该值不受限制。该选项可以采用选项类型可能的任何值。
     *
     * SANE_CONSTRAINT_RANGE 1 	此约束仅适用于整数和定点值的期权。它将期权价值限制在可能量化的数字范围内。
     *                          选项描述符成员constraint.range指向SANE_Range类型的范围。这种类型如下所示：
     *
     *        typedef struct
     *        {
     *          SANE_Word min;
     *          SANE_Word max;
     *          SANE_Word quant;
     *        }
     *        SANE_Range;
     *
     * 根据选项值类型（SANE_TYPE_INT或SANE_TYPE_FIXED）解释此结构中的所有三个成员。成员min和max分别指定最小值和最大值。
     * 如果成员quantit非零，则它指定量化值。如果l是最小值，u是最大值，q是范围的（非零）量化，那么对于k的所有非负整数值，合法值是v = k * q + 1，从而v <=你
     *
     * SANE_CONSTRAINT_WORD_LIST	2 	此约束仅适用于整数和定点值的期权。它将选项值限制为数字值列表。
     × 选项描述符成员 constraint.word_list指向枚举合法值的单词列表。该列表中的第一个元素是一个整数（SANE_Int），它指定列表的长度（不计算长度本身）。
     * 列表中的其余元素根据选项值的类型（SANE_TYPE_INT或SANE_TYPE_FIXED）进行解释。
     *
     * SANE_CONSTRAINT_STRING_LIST	3 	此约束仅适用于字符串值的选项。它将选项值限制为字符串列表。
     * 选项描述符成员 constraint.string_list指向以NULL结尾的字符串列表，这些列表枚举了选项值的合法值。
     */
    SANE_Constraint_Type constraint_type;

    union
      {
	const SANE_String_Const *string_list;	/**< NULL-terminated list */
	const SANE_Word *word_list;	            /**< first element is list-length */
	const SANE_Range *range;
      }
    constraint;
  }
SANE_Option_Descriptor;

typedef enum
  {
    SANE_ACTION_GET_VALUE = 0,
    SANE_ACTION_SET_VALUE,
    SANE_ACTION_SET_AUTO
  }
SANE_Action;

typedef enum
  {
    SANE_FRAME_GRAY,	/**< band covering human visual range */
    SANE_FRAME_RGB,	    /**< pixel-interleaved red/green/blue bands */
    SANE_FRAME_RED,	    /**< red band only */
    SANE_FRAME_GREEN,	/**< green band only */
    SANE_FRAME_BLUE 	/**< blue band only */
  }
SANE_Frame;

/* push remaining types down to match existing backends */
/* these are to be exposed in a later version of SANE */
/* most front-ends will require updates to understand them */
#if 0
#define SANE_FRAME_TEXT  0x0A  /**< backend specific textual data */
#define SANE_FRAME_JPEG  0x0B  /**< complete baseline JPEG file */
#define SANE_FRAME_G31D  0x0C  /**< CCITT Group 3 1-D Compressed (MH) */
#define SANE_FRAME_G32D  0x0D  /**< CCITT Group 3 2-D Compressed (MR) */
#define SANE_FRAME_G42D  0x0E  /**< CCITT Group 4 2-D Compressed (MMR) */

#define SANE_FRAME_IR    0x0F  /**< bare infrared channel */
#define SANE_FRAME_RGBI  0x10  /**< red+green+blue+infrared */
#define SANE_FRAME_GRAYI 0x11  /**< gray+infrared */
#define SANE_FRAME_XML   0x12  /**< undefined schema */
#endif

/**
 * 扫描参数类型
 * 成员格式指定要返回的下一帧的格式。SANE_Frame类型的可能值在表9中进行了描述
 *
 * 帧格式（SANE_Frame）
 *
 * 符号 	        码 	描述
 * 
 * SANE_FRAME_GRAY 	0 	乐队覆盖了人类的视觉范围。
 * SANE_FRAME_RGB 	1 	像素交错的红色/绿色/蓝色带。
 * SANE_FRAME_RED 	2 	红色/绿色/蓝色图像的红色带。
 * SANE_FRAME_GREEN 3 	红色/绿色/蓝色图像的绿色带。
 * SANE_FRAME_BLUE 	4 	红色/绿色/蓝色图像的蓝色带。
 *
 *
 * 当且仅当当前正在获取的帧（或者如果当前帧不存在时，下一个将要获取的帧）是多帧图像的最后一个帧（例如，当前帧是当前帧）时，
 * 成员last_frame设置为SANE_TRUE。红色，绿色，蓝色图像的蓝色成分）。
 * 
 * 成员行指定框架包含多少条扫描线。如果此值为-1，则行数是先验未知的，并且前端应调用sane_read（），直到返回状态SANE_STATUS_EOF为止。
 * 成员bytes_per_line指定组成一条扫描线的字节数。
 * 成员深度指定每个样本的位数。
 * 成员pixels_per_line指定组成一条扫描线的像素数。
 * 
 * 假设B是帧中的通道数，则位深度d（由memer depth给出）和每行像素数n（由此memberpixels_per_line给出）与c（每行字节数）相关（由成员bytes_per_line给出 ）如下：
 * 
 * c> = \ left {ll B * \ lfloor（n + 7）/ 8 \ rfloor如果d = 1（1）B * n * d / 8如果d> 1 \ right。
 * 
 * 请注意，每行的字节数可以大于此方程式右侧施加的最小值。前端必须能够正确处理此类``填充''图像格式。
 */

typedef struct
  {
    SANE_Frame format;
    SANE_Bool last_frame;
    SANE_Int bytes_per_line;
    SANE_Int pixels_per_line;
    SANE_Int lines;
    SANE_Int depth;
  }
SANE_Parameters;

struct SANE_Auth_Data;

#define SANE_MAX_USERNAME_LEN	128
#define SANE_MAX_PASSWORD_LEN	128

/**
 * 认证功能类型
 * 三个参数传递给授权函数： 
 * resource是一个字符串，用于指定需要授权的资源的名称。前端应使用此字符串来构建一个要求用户名和密码的用户提示。
 * 用户名 和密码参数是（指针）的阵列 SANE_MAX_USERNAME_LEN和SANE_MAX_PASSWORD_LEN 字符，
 * 分别授权调用应将输入的用户名和密码放在这些数组中。返回的字符串 必须以ASCII-NUL终止。
 */
typedef void (*SANE_Auth_Callback) (SANE_String_Const resource,
				    SANE_Char *username,
				    SANE_Char *password);





//////////////////////////////////////// //////////////////////////////////////////////////////////////
// sane_init（）和 sane_exit（）分别初始化和退出后端，所有其他调用必须在初始化之后和退出后端之前执行

/**
 * <pre>
 * 代码流
 * 
 * -sane_init()
 *      -pick desired device, possibly by using sane_get_devices()
 *      -sane_open()
 *          //device setup
 *          -use: 
 *                  sane_get_option_description()
 *                  sane_control_option()
 *          repeatedly to configure device as desired
 *
 *          //image acquisition
 *          -sane_start()
 *          -use: 
 *                  sane_get_parameters()
 *                  sane_read()
 *          repeatedly until read returns EOF
 *          -go back to sane_start() if more frames desired
 *      -sane_close()
 * -sane_exit()
 *
 * 
 * 流程详解：
 * 可以在调用sane_init（）之后的任何时间调用 函数sane_get_devices（）。它返回呼叫时已知的设备列表。
 * 由于某些设备可能已打开或关闭，或者远程主机可能在不同的呼叫之间启动或关闭，因此该列表可能会随时间变化。
 * 应当注意，此操作可能相对较慢，因为它需要联系所有已配置的设备（其中一些可能在远程主机上）。
 * 因此，前端可能希望为用户提供直接选择所需设备的功能，而无需调用此功能。
 * 
 * 选择设备后，可通过调用sane_open（）将其打开 。可以在任何给定时间打开多个设备。SANE后端不得在任何给定时间对可以打开多少个设备施加人为约束。
 * 
 * 可以使用功能sane_get_option_descriptor（）和 sane_control_option（）通过相应的设备句柄设置打开的设备。
 * 设置设备时，可以自由地混合使用选项描述符以及设置和读取选项值。前端通常会在开始时读出所有可用选项，
 * 然后建立一个对话框（图形或命令行选项列表），以控制可用选项。应当注意，对于给定的句柄，选项的数量是固定的。
 * 但是，随着选项的设置，其他选项可能会变为活动或非活动状态。因此，在设置选项之后，可能需要重新读取一些或所有选项描述符。
 * 设置设备时，也可以拨打电话 sane_get_parameters（）以获取图像获取开始后图像参数的估算值。
 * 
 * 通过调用sane_set_io_mode（）可以将设备句柄置于阻塞或非阻塞模式。
 * 要求设备支持阻止模式（这是默认模式），但是对于诸如UNIX之类的操作系统，强烈建议支持非阻止I / O。
 * 
 * 正确设置设备后，可以通过调用sane_start（）来启动图像获取。后端此时会计算出确切的图像参数。
 * 因此，将来对sane_get_parameters（）的调用 将返回确切的值，而不是估计值。
 * SANE API未指定是在这一点上还是在第一次调用sane_read（）时开始物理图像获取。
 * 如果需要无阻塞的I / O和/或选择样式的接口，则前端可能会在此时尝试调用 sane_set_io_mode（）和/或sane_get_select_fd（）。
 * 如果后端不支持请求的操作，则这些功能中的任何一个都可能失败。
 * 
 * 通过重复调用sane_read（）来收集图像数据。最终，此函数将返回文件结束状态（SANE_STATUS_EOF）。
 * 这指示当前帧的结束。如果前端需要其他帧（例如，红色/绿色/蓝色图像或多个图像中的各个通道），则可以再次调用sane_start（）。
 * 一旦获取了所有所需的帧，就必须调用函数sane_cancel（）。也可以在任何其他时间调用此操作以取消挂起的操作。
 * 请注意，即使最后一次读取操作返回了SANE_STATUS_EOF，也必须调用sane_cancel（）。
 * 
 * 使用设备完成操作后，应通过调用sane_close（）关闭句柄 。最后，在退出应用程序之前，必须调用函数sane_exit（）。
 * 重要的是不要忘记调用此函数，否则可能会导致某些资源（例如临时文件或锁）无人认领。
 *
 *
 *
 * 其他选项
 * 尽管大多数后端选项都是完全自描述的，但在某些情况下，用户界面可能希望对某些选项进行特殊处理。
 * 例如，扫描区域通常由四个选项定义，这些选项指定区域的左上角和右下角。使用图形用户界面，迫使用户键入这四个数字将很繁琐。
 * 相反，大多数此类界面都希望向用户呈现扫描仪表面的预览（低分辨率扫描），并让用户通过将矩形拖动到所需位置来选择扫描区域。
 * 因此，SANE API指定了少数具有明确定义含义的选项名称。
 *
 *
 * 选件编号计数
 * 选项号0的名称为空字符串。此选项的值的类型为SANE_TYPE_INT，它指定给定设备可用的选项总数（计数包括选项号0）。
 * 这意味着有两种计算可用选项数量的方法：前端可以循环从一个选项开始的所有选项编号，
 * 直到 sane_get_option_descriptor（）返回NULL，或者前端可以直接读出选项编号0的值。
 *
 *
 * 扫描分辨率选项
 * 选项分辨率用于选择应获取图像的分辨率。此选项的类型为 SANE_TYPE_INT或SANE_TYPE_FIXED。单位是 SANE_UNIT_DPI（点/英寸）。
 * 此选项不是强制性的，但是如果后端确实支持它，则它必须以与上述定义一致的方式来实现。
 *
 *
 * 预览模式选项
 * 前端使用布尔选项预览来通知后端何时应针对速度而非质量（``预览模式''）优化图像采集。
 * 设置为SANE_TRUE时，预览模式有效；设置为SANE_FALSE时，图像采集应以普通质量模式进行。
 * 此选项的设置不得影响任何其他选项。也就是说，就其他选项而言，预览模式完全没有副作用。
 * 后端可以假定前端将适当地设置预览模式的扫描分辨率（通过option resolution）。
 * 后端可以随意覆盖 分辨率 可以选择自己的预览模式，但建议尽可能将其保留在前端。
 * 此选项不是强制性的，但是如果后端确实支持它，则它必须以与上述定义一致的方式来实现。
 *
 *
 * 扫描区域选项
 * 四个最重要的众所周知的选项是定义扫描区域的选项。扫描区域由指定左上角和右下角的两个点（x / y坐标对）定义。
 * 这在图5中示出。请注意，坐标系的原点位于传感器所看到的扫描表面的左上角（通常是用户看到的扫描表面的镜像）。
 * 因此，左上角是横坐标和纵坐标值同时最小的角，右下角是横坐标和纵坐标值近似最大的角。
 * 如果此坐标系对于给定的设备而言不是自然的，则后端的工作就是执行必要的转换。
 *
 × 扫描区域选项
 *                                              → x
 *       0 --------------------------------------
 *       |             scan surface             |
 *       |                                      |
 *       |                                      |
 *       |      top-left                        |
 *       |          -----------------           |
 *       |          |  scan area    |           |
 *       |          -----------------           |
 *       |                       bottom-right   |
 *       |                                      |
 *       |                                      |
 *  ↓ y  |---------------------------------------
 *
 *
 * 下表列出了定义扫描区域的四个选项的名称：
 * 名称 	描述
 * tl-x 	左上角x坐标值
 * tl-y 	左上y坐标值
 * br-x 	右下x坐标值
 * br-y 	右下y坐标值
 * 
 * 关于这些选项，前端和后端应遵循几个规则：
 * 
 *     后端必须将像素（SANE_UNIT_PIXEL）或毫米（SANE_UNIT_MM）的单位附加到这些选项。所有四个选项的单位必须相同。
 *     每当有意义时，后端应将范围或单词列表约束附加到这些选项。
 *     前端可以通过首先检查选项是否具有关联的范围约束来确定扫描表面的大小。
 *     如果存在范围或单词列表约束，则前端可以采用x和y选项范围约束之一的最小值和最大值来确定扫描表面大小。
 *     前端必须正确运行，而缺少任何或所有这些选项。
 *
 *
 *
 * 网络协议
 * SANE接口的设计，方便图像采集设备的网络接入。特别是，
 * 大多数SANE实施都有望支持网络后端（网络客户端）和相应的网络守护程序（网络服务器），
 * 该网络后台程序允许通过网络连接访问图像采集设备。网络访问在以下几种情况下很有用：
 *      1）为了提供给那些无法进入正规的用户资源受控访问。例如，用户可能想访问没有帐户的主机上的设备。
 *         使用网络协议，可以允许某些用户访问扫描仪而无需给予他们对系统的完全访问权限。
 * 
 *      2) 即使在本地情况下，通过网络守护程序控制访问也会很有用：例如，某些后端可能需要root特权才能访问设备。
 *          系统管理员可以将SANE网络守护程序安装为setuid-root，而不是将每个前端都安装为setuid-root。
 *          这使普通用户可以通过SANE守护程序访问特权设备（该守护程序可能比简单的setuid方法支持更细粒度的访问控制机制）。
 *          这具有额外的好处，即系统管理员只需要信任SANE守护程序，而不是可能需要访问特权设备的每个前端。
 *          网络访问提供了可用的图像采集设备的普及之感。例如，在局域网环境中，这允许用户登录到任何计算机上，
 *          并可以方便地访问网络上任何计算机可用的任何资源（受权限限制）。
 *
 *      3）对于使用时不需要物理访问的设备（例如摄像机），网络访问允许用户控制和使用这些设备而无需物理接近。
 *          实际上，如果将此类设备连接到Internet，则可以从世界上任何地方进行访问。
 * 
 * 
 * 在设计本章中描述的网络协议时，要牢记以下目标：
 * 
 *     1. 图像传输应该高效（编码开销较低）。
 *     2. 在客户端访问选项描述符必须高效（因为这是非常常见的操作）。
 *     3. 其他操作（如设置或查询选项的值）对性能的要求不高，因为它们通常需要明确的用户操作。
 *     4. 网络协议应该简单易行，可以在任何主机体系结构和任何编程语言上实现。
 * 
 * SANE协议可以在任何提供可靠数据传输的传输协议上运行。尽管SANE没有指定特定的传输协议，但是可以预期TCP / IP将成为最常用的协议之一。
 * 
 * </pre>
 */

//////////////////////////////////////// /////////////////////////////////////////////////////////////



/**
 * 初始化后端
 * 必须先调用此函数，然后才能调用任何其他SANE函数。
 * 如果此功能未先叫或SANE后端的行为都是不确定如果返回的状态代码sane_init不同于 SANE_STATUS_GOOD。
 * 后端的版本代码以version_code指向的值返回。如果该指针为 NULL，则不返回任何版本代码。
 * 参数授权是指向后端要求对特定资源进行身份验证时调用的函数的指针，或者如果前端不支持身份验证则为NULL。
 *
 * 后端可以响应以下任何调用来调用授权功能：
 *
 *    sane_open，sane_control_option，sane_start
 *
 * 如果后端在没有授权功能的情况下初始化，那么后端本身无法处理的授权请求将自动失败，并且可能会阻止用户访问受保护的资源。
 * 鼓励后端实施不需要用户协助的身份验证方法。例如，在通过登录过程验证用户身份的多用户系统上，后端可以根据资源和用户名自动查找适当的密码。
 */
extern SANE_Status sane_init (SANE_Int * version_code,
			      SANE_Auth_Callback authorize);

/**
 * @brief 
 * 退出后端
 *
 * <pre>
 * @details 
 * 必须调用此函数以终止使用后端。
 * 该函数将首先关闭所有可能仍处于打开状态的设备句柄（建议通过调用sane_close（）显式关闭设备句柄，但要求后端在调用此函数时释放所有资源）。
 * 此函数返回后，不能调用sane_init（）以外的任何函数（无论sane_exit（）返回的状态值如何。忽略调用此函数可能会导致某些资源无法正确释放。
 * </pre>
 *
 * @param   a  变量a
 * @param   a  变量a
 * @return void
 * @retval 
 * 1: result1
 * 2: result2
 */
extern void sane_exit (void);

/**
 * 此功能可用于查询可用设备的列表。
 * 如果函数成功执行，它将 在* device_list中存储指向NULL终止数组的指针，该数组指向SANE_Device结构。
 * 保证返回的列表保持不变并有效，直到（a）对该函数的另一次调用或（b）对sane_exit（）的调用。
 * 可以重复调用此功能以检测何时有新设备可用。如果参数local_only为true，则仅返回本地设备（直接连接到运行SANE的计算机的设备）。
 * 如果为假，则设备列表包括SANE库可访问的所有远程设备。
 *
 * 如果没有足够的内存，此功能可能会因SANE_STATUS_NO_MEM而失败。
 * 
 * 后端实施说明
 * SANE不需要在执行sane_open（）调用之前调用此函数 。设备名称可以由用户明确指定，这将导致不必要且不希望先调用此功能。
 */
extern SANE_Status sane_get_devices (const SANE_Device *** device_list,
				     SANE_Bool local_only);

/**
 * 此功能用于建立到特定设备的连接。
 * 要打开的设备的名称在参数name中传递 。如果调用成功完成，则在* h中返回设备的句柄。
 * 在特殊情况下，将零长度字符串指定为设备会请求打开第一个可用设备（如果有的话）。
 * 此功能可能因以下状态代码之一而失败。
 *
 *    SANE_STATUS_DEVICE_BUSY：
 *    设备当前正忙（其他人正在使用）。
 *
 *    SANE_STATUS_INVAL：
 *    设备名称无效。
 *
 *    SANE_STATUS_IO_ERROR：
 *    与设备通信时发生错误。
 *
 *    SANE_STATUS_NO_MEM：
 *    可用的内存量不足。
 *
 *    SANE_STATUS_ACCESS_DENIED：
 *    由于身份验证不充分或无效，因此无法访问设备。
 */
extern SANE_Status sane_open (SANE_String_Const devicename,
			      SANE_Handle * handle);

/**
 * 此函数终止在参数h中传递的设备句柄与其表示的设备之间的关联。
 * 如果设备当前处于活动状态，则首先执行对sane_cancel（）的调用。此函数返回后，不能再使用句柄h。
 */
extern void sane_close (SANE_Handle handle);

/**
 * 此函数用于访问选项描述符。该函数返回由句柄h表示的设备的选项号n的选项描述符。
 * 选项号0保证是有效的选项。它的值是一个整数，它指定可用于设备句柄h的选项数量（计数包括选项0）。
 * 如果n不是有效的选项索引，则该函数返回NULL。保证返回的选项描述符在关闭设备之前一直保持有效（并在返回的地址处）。
 */
extern const SANE_Option_Descriptor *
  sane_get_option_descriptor (SANE_Handle handle, SANE_Int option);

/**
 * 此功能用于设置或查询由句柄h表示的设备的选件编号n的当前值。
 * 选项a的控制方式由参数a指定 。该参数的可能值将在下面更详细地描述。
 * 该选项的值通过参数v传递 。它是指向保存选项值的内存的指针。
 * v指向的内存区域必须足够大以容纳整个选项值（由相应选项描述符中的成员大小确定）。
 * 该规则的唯一例外是，在设置字符串选项的值时，参数v指向的字符串可能会更短，因为在遇到 字符串中的第一个NUL终止符时，后端将停止读取选项值。
 * 如果参数i不为NULL，则将设置* i的值以提供有关如何满足请求的详细信息。该参数的含义将在下面更详细地描述。
 *
 * 通过调用此函数影响选项的方式由参数a控制，参数a是SANE_Action类型的值 。可能的值及其含义在表7中描述。
 *
 * 符号 	                码 	描述
 * 
 * SANE_ACTION_GET_VALUE  	0 	获取当前的期权价值。
 * 
 * SANE_ACTION_SET_VALUE 	1 	设置选项值。如果无法精确设置通过参数v传递的选项值，则后端可能会对其进行修改。
 * 
 * SANE_ACTION_SET_AUTO     2 	打开自动模式。后端或设备将自动选择适当的值。此模式将保持有效，直到被显式设置值请求覆盖为止。
 *                              在这种情况下，参数v的值将被完全忽略，并且可以为NULL。
 *
 *
 * 通过SANE_ACTION_SET_VALUE的操作值设置一个值后 ，在* i中返回有关如何满足请求的其他信息（如果i为非NULL）。返回的值是一个位集，可以包含表8中描述的值的任何组合。
 * 
 * 符号 	                    码 	描述
 * 
 * SANE_INFO_INEXACT 	        1 	当设置选项值导致选择的值与请求的值不完全匹配时，将返回此值。
 *                                  例如，如果扫描仪只能以30dpi的增量调整分辨率，则将分辨率设置为307dpi可能会导致实际设置为300dpi。
 *                                  发生这种情况时，* i中返回的位集具有此成员集。另外，修改了选项值以反映后端使用的实际（四舍五入）值。
 *                                  请注意，字符串也可以使用不精确的值。后端可以选择将字符串``舍入''到最接近的匹配合法字符串以获取受约束的字符串值。
 * 
 * SANE_INFO_RELOAD_OPTIONS  	2 	选项的设置可能会影响一个或多个 其他选项的价值或可用性。
 *                                  发生这种情况时，SANE后端会在* i中设置此成员，以指示应用程序应重新加载所有选项。
 *                                  仅当更改了至少一个选项时，才能设置此成员。
 * 
 * SANE_INFO_RELOAD_PARAMS      4 	选项的设置可能会影响参数值（请参见sane_get_parameters（））。
 *                                  如果设置选项影响参数值，则该成员将在* i中设置。请注意，即使参数没有实际更改，也可以设置该成员。
 *                                  但是，可以确保不设置此成员就不会更改参数。
 *
 * 此功能可能因以下状态代码之一而失败。
 *
 *    SANE_STATUS_UNSUPPORTED：
 *
 *    指定的句柄和选项号不支持该操作。
 *
 *    SANE_STATUS_INVAL：
 *
 *    选项值无效。
 *
 *    SANE_STATUS_IO_ERROR：
 *
 *    与设备通信时发生错误。
 *
 *    SANE_STATUS_NO_MEM：
 *
 *    可用的内存量不足。
 *
 *    SANE_STATUS_ACCESS_DENIED：
 *
 *    由于身份验证不充分或无效，因此无法访问该选件。
 *
 */
extern SANE_Status sane_control_option (SANE_Handle handle, SANE_Int option,
					SANE_Action action, void *value,
					SANE_Int * info);

/**
 * 此功能用于获取当前的扫描参数。
 * 在开始扫描（调用sane_start（））与完成请求之间，保证返回的参数是准确的。
 * 在该窗口之外，返回值是在调用sane_start（）时参数将尽力而为的估计 。
 * 例如，在实际开始扫描之前调用此函数可以例如估算出扫描图像的大小。
 * 传递给此函数的参数是应为其获取参数的设备的句柄h，以及指向参数结构的指针p。参数结构在下面更详细地描述。
 * 扫描参数以SANE_Parameters类型的结构返回 。
 */
extern SANE_Status sane_get_parameters (SANE_Handle handle,
					SANE_Parameters * params);

/**
 * 此功能启动从手柄h表示的设备获取图像。
 * 此功能可能因以下状态代码之一而失败。
 * 
 *     SANE_STATUS_CANCELLED：
 *     通过调用sane_cancel取消了该操作。
 * 
 *     SANE_STATUS_DEVICE_BUSY：
 *     设备忙。该操作应稍后重试。
 * 
 *     SANE_STATUS_JAMMED：
 *     文档进纸器被卡住。
 * 
 *     SANE_STATUS_NO_DOCS：
 *     文件进纸器中没有文件。
 * 
 *     SANE_STATUS_COVER_OPEN：
 *     扫描仪盖板打开。
 * 
 *     SANE_STATUS_IO_ERROR：
 *     与设备通信时发生错误。
 * 
 *     SANE_STATUS_NO_MEM：
 *     可用的内存量不足。
 * 
 *     SANE_STATUS_INVAL：
 *     无法使用当前选项集开始扫描。前端应该重新加载选项描述符，就像 SANE_INFO_RELOAD_OPTIONS已从对sane_control_option（）的调用返回 ，因为设备的功能可能已更改。
 */
extern SANE_Status sane_start (SANE_Handle handle);

/**
 * 此功能用于从手柄h代表的设备中读取图像数据。
 * 参数buf是一个指向至少maxlen个字节长的存储区的指针。返回的字节数存储在* len中。当返回SANE_STATUS_GOOD以外的状态时，后端必须将此值设置为零。当调用成功，返回的字节的数目可以在从0到的范围内的任何地方的maxlen字节。
 * 如果在没有可用数据时调用此函数，则可能发生两种情况之一，具体取决于对句柄h生效的I / O模式。
 * 
 *     如果设备处于阻止I / O模式（默认模式），则调用将阻止直到至少一个数据字节可用（或直到发生某些错误）为止。
 * 
 *      
 *     如果设备处于非阻塞I / O模式，则调用立即返回，状态为SANE_STATUS_GOOD，并且 * len设置为零。
 * 
 * 可以通过调用sane_set_io_mode（）来设置 句柄h的I / O模式。
 * 
 * 此功能可能因以下状态代码之一而失败。
 * 
 *     SANE_STATUS_CANCELLED：
 *     通过调用sane_cancel取消了该操作。
 * 
 *     SANE_STATUS_EOF：
 *     当前帧没有更多数据。
 * 
 *     SANE_STATUS_JAMMED：
 *     文档进纸器被卡住。
 * 
 *     SANE_STATUS_NO_DOCS：
 *     文件进纸器中没有文件。
 * 
 *     SANE_STATUS_COVER_OPEN：
 *     扫描仪盖板打开。
 * 
 *     SANE_STATUS_IO_ERROR：
 *     与设备通信时发生错误。
 * 
 *     SANE_STATUS_NO_MEM：
 *     可用的内存量不足。
 * 
 *     SANE_STATUS_ACCESS_DENIED：
 *     由于身份验证不充分或无效，因此无法访问设备。
 */
extern SANE_Status sane_read (SANE_Handle handle, SANE_Byte * data,
			      SANE_Int max_length, SANE_Int * length);

/**
 * 此功能用于立即或尽快取消由句柄h表示的设备的当前挂起的操作 。
 * 可以随时调用此函数（只要句柄h是有效的句柄），但通常只影响长时间运行的操作（例如图像获取）。
 * 异步调用此函数是安全的（例如，从信号处理程序中）。重要的是要注意，此操作的完成并不意味着当前挂起的操作已被取消。
 * 它仅保证取消已启动。取消仅在取消的呼叫返回时完成（通常状态值为SANE_STATUS_CANCELLED）。
 * 由于SANE API不需要重新输入任何其他操作，因此这意味着前端一定不能 调用任何其他操作，直到返回已取消的操作。
 */
extern void sane_cancel (SANE_Handle handle);

/**
 * 此功能用于设置手柄h的I / O模式。
 * I / O模式可以是阻塞或非阻塞。如果参数m为 SANE_TRUE，则将模式设置为非阻塞模式，否则将其设置为阻塞模式。仅在对sane_start（）的调用完成后才能调用此函数 。
 * 默认情况下，新打开的句柄以阻止模式运行。后端可以选择不支持非阻塞I / O模式。在这种情况下，将返回状态值SANE_STATUS_UNSUPPORTED。
 * 所有后端都必须支持阻塞I / O，因此可以确保将参数m设置为SANE_FALSE调用此函数。
 * 
 * 此功能可能因以下状态代码之一而失败：
 * 
 *     SANE_STATUS_INVAL：
 * 
 *     没有图像采集正在等待。
 * 
 *     SANE_STATUS_UNSUPPORTED：
 * 
 *     后端不支持请求的I / O模式。
 */
extern SANE_Status sane_set_io_mode (SANE_Handle handle,
				     SANE_Bool non_blocking);

/**
 * 此函数用于获取句柄h的（特定于平台的）文件描述符
 * 当且仅当图像数据可用时（即，对sane_read（）的调用将返回至少一个字节的数据），该文件描述符才可读。如果调用成功完成，则在* fd中返回选择文件描述符。
 * 仅在执行对sane_start（）的调用 并且已保证返回的文件描述符在当前图像获取期间保持有效
 *（即，直到再次调用sane_cancel（）或sane_start（）或直到sane_read（）返回状态 SANE_STATUS_EOF）。
 * 实际上，后端必须保证在下一个sane_read（）调用返回SANE_STATUS_EOF时关闭返回的选择 文件描述符。
 * 这是确保应用程序可以检测到何时发生这种情况而不必实际调用sane_read（）所必需的。
 * 
 * 后端可以选择不支持此操作。在这种情况下，该函数将返回状态码 SANE_STATUS_UNSUPPORTED。
 * 
 * 请注意，返回的文件描述符支持的唯一操作是与主机操作系统相关的测试，以确定文件描述符是否可读
 * （例如，可以在UNIX下使用select（） 或poll（）来实现此测试）。如果对文件描述符执行任何其他操作，则后端的行为将变得不可预测。
 * 一旦文件描述符发出``可读''状态的信号，它将保持该状态，直到执行对sane_read（）的调用为止。
 * 由于许多输入设备非常慢，因此强烈建议支持此操作，因为它允许应用程序在进行图像采集时执行其他工作。
 * 
 * 此功能可能因以下状态代码之一而失败：
 * 
 *     SANE_STATUS_INVAL：
 * 
 *     没有图像采集正在等待。
 * 
 *     SANE_STATUS_UNSUPPORTED：
 * 
 *     后端不支持此操作。
 */
extern SANE_Status sane_get_select_fd (SANE_Handle handle,
				       SANE_Int * fd);

/**
 * 此功能可用于将SANE状态代码转换为可打印的字符串。
 * 返回的字符串是形成完整句子的单行文本，但没有结尾句号（句号）。该函数保证永远不会返回NULL。返回的指针至少在下一次调用此函数之前是有效的。
 */
extern SANE_String_Const sane_strstatus (SANE_Status status);

#ifdef __cplusplus
}
#endif 


#endif /* sane_h */
