/*
 * Compile me with:
 *   gcc -o spawn spawn.c $(pkg-config --cflags --libs gtk+-2.0)
 */
#include <gtk/gtk.h>
 
typedef struct _Data Data;
struct _Data
{
    /* Buffers that will display output */
    GtkTextBuffer *out;
    GtkTextBuffer *err;
 
    /* Progress bar that will be updated */
    GtkProgressBar *progress;
 
    /* Timeout source id */
    gint timeout_id;
};
 
static void
cb_child_watch( GPid  pid,
                gint  status,
                Data *data )
{
    /* Remove timeout callback */
    g_source_remove( data->timeout_id );
 
    /* Close pid */
    g_spawn_close_pid( pid );
}
 
static gboolean
cb_out_watch( GIOChannel   *channel,
              GIOCondition  cond,
              Data         *data )
{
    gchar *string;
    gsize  size;
 
    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }
 
    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    gtk_text_buffer_insert_at_cursor( data->out, string, -1 );
    g_free( string );
 
    return( TRUE );
}
 
static gboolean
cb_err_watch( GIOChannel   *channel,
              GIOCondition  cond,
              Data         *data )
{
    gchar *string;
    gsize  size;
 
    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }
 
    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    gtk_text_buffer_insert_at_cursor( data->err, string, -1 );
    g_free( string );
 
    return( TRUE );
}
 
static gboolean
cb_timeout( Data *data )
{
    /* Bounce progress bar */
    gtk_progress_bar_pulse( data->progress );
 
    return( TRUE );
}
 
static void
cb_execute( GtkButton *button,
            Data      *data )
{
    GPid        pid;
    gchar      *argv[] = { "./helper", NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;
 
    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
                                    G_SPAWN_DO_NOT_REAP_CHILD, NULL,
                                    NULL, &pid, NULL, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return;
    }
 
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, data );
 
    /* Create channels that will be used to read data from pipes. */
#ifdef G_OS_WIN32
    out_ch = g_io_channel_win32_new_fd( out );
    err_ch = g_io_channel_win32_new_fd( err );
#else
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );
#endif
 
    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, data );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_err_watch, data );
 
    /* Install timeout fnction that will move the progress bar */
    data->timeout_id = g_timeout_add( 100, (GSourceFunc)cb_timeout, data );
}
 
int
main( int    argc,
      char **argv )
{
    GtkWidget *window,
              *table,
              *button,
              *progress,
              *text;
    Data      *data;
 
    data = g_slice_new( Data );
 
    gtk_init( &argc, &argv );
 
    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_default_size( GTK_WINDOW( window ), 400, 300 );
    g_signal_connect( G_OBJECT( window ), "destroy",
                      G_CALLBACK( gtk_main_quit ), NULL );
 
    table = gtk_table_new( 2, 2, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE( table ), 6 );
    gtk_table_set_col_spacings( GTK_TABLE( table ), 6 );
    gtk_container_add( GTK_CONTAINER( window ), table );
 
    button = gtk_button_new_from_stock( GTK_STOCK_EXECUTE );
    g_signal_connect( G_OBJECT( button ), "clicked",
                      G_CALLBACK( cb_execute ), data );
    gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 0, 1,
                      GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
 
    progress = gtk_progress_bar_new();
    data->progress = GTK_PROGRESS_BAR( progress );
    gtk_table_attach( GTK_TABLE( table ), progress, 1, 2, 0, 1,
                      GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
 
    text = gtk_text_view_new();
    data->out = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text ) );
    gtk_table_attach( GTK_TABLE( table ), text, 0, 1, 1, 2,
                      GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0 );
 
    text = gtk_text_view_new();
    data->err = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text ) );
    gtk_table_attach( GTK_TABLE( table ), text, 1, 2, 1, 2,
                      GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0 );
 
    gtk_widget_show_all( window );
 
    gtk_main();
 
    g_slice_free( Data, data );
 
    return( 0 );
}
