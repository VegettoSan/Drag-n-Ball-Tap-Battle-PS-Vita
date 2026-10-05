package android.util;

public final class Log { public static int e(String tag,String value){System.err.println(tag+": "+value);return 0;} public static int d(String tag,String value){System.out.println(tag+": "+value);return 0;} }
