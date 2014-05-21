/**
 * @file
 *   JS behavior for Paragraph chunk type. Instantiates the Pen editor.
 */

(function ($){
  Drupal.behaviors.penEditor = {
    attach: function(context, settings) {
      // Initialize editor for this Paragraph chunk
      $('.p-chunk').each(function() {
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
      // .live('keyup', function() {
      //   // Each keystroke, copy the data back into the form item so it gets saved
      //   // when the user submits the form.
      //   console.log('#edit-' + classPrepend.substring(1, classPrepend.length - 2) + 'und-' + classPrepend.substring(classPrepend.length - 2) + 'configuration-p-p');
      //   $('#edit-' + classPrepend.substring(1, classPrepend.length - 2) + 'und-' + classPrepend.substring(classPrepend.length - 2) + 'configuration-p-p').val($(this).html());
      // });
    }
  };
})(jQuery);
