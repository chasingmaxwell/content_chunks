/**
 * @file
 *   JS behavior for Paragraph chunk type. Instantiates the Pen editor.
 */

(function ($){
  Drupal.behaviors.penEditor = {
    attach: function(context, settings) {
      // Initialize editor for each Paragraph chunk
      $('.p-chunk').each(function(){
        var editor,
            editorConfig;

        editorConfig = {
          class: 'pen-editor',
          debug: true,
          editor: this,
          textarea: '<textarea name="content"></textarea>', // fallback for old browsers
          list: ['bold', 'italic', 'underline', 'createlink']
        };
        editor = new Pen(editorConfig);
      });
    }
  };
})(jQuery);
