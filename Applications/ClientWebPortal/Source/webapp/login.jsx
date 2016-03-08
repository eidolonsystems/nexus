require(['react', 'react-dom', 'ReactRouter.min', 'jquery', 'TimerMixin'],
  function(React, ReactDOM, ReactRouter, $, TimerMixin) {
    var Router = ReactRouter.Router;
    var Route = ReactRouter.Route;
    var Link = ReactRouter.Link;
    var logoStyle = {};
    var LoginPage = React.createClass({
      mixins: [TimerMixin],
      render: function() {
        return (
          <div className="login_page">
            <img ref="logo" id="logo" src={this.state.logo_src} loop="infinite"
            alt="Spire Trading Logo" style={logoStyle} />
            <form ref="login_form" id="login_form" onSubmit={this.handleSubmit}>
              <input 
                autoFocus 
                className="login_input" 
                id="login_username" 
                type="text" 
                name="login_username" 
                placeholder="Username" 
                ref="login_username"
                value={this.state.username}
                onChange={this.handleUsernameChange} /><br/>
              <input 
                className="login_input" 
                id="login_password" 
                type="password"
                name="login_password" 
                placeholder="Password"
                ref="login_password"
                value={this.state.password}
                onChange={this.handlePasswordChange} /><br/>
              <input 
                id="login_submit"
                className={this.state.submitted ? "inactive" : ""}
                type="submit"
                ref="login_submit"
                value="Login" />
              <p className="error_messages"> {this.state.errorMessages} </p>
            </form>
          </div>);
      },
      getInitialState: function() {
        return {
          username: ''
          , password: ''
          , submitted : false
          , logo_src: 'img/spire_white.png'
          , errorMessages: ''
        };
      },
      componentDidMount: function () {
        console.log("submitted: " + this.state.submitted);
      },
      componentWillUpdate: function () {
      },
      componentWillUnmount: function() {
      },
      handleSubmit: function(e) {
        e.preventDefault();
        this.setState({ submitted: true });
        console.log("submitted after handleSubmit func: " + this.state.submitted);
        console.log("Handle submit is working!");
        console.log("image src before: " + this.state.logo_src);
        var submitted_username = this.state.username.trim();
        var submitted_password = this.state.password.trim();
        if (!submitted_username) {
          console.log("No username or password");
          return;
        }
        this.setState({username: ''});
        this.setState({password: ''});
        console.log("Entered Username is: " + submitted_username);
        console.log("Entered Password is: " + submitted_password);
        var jqxhr = $.ajax(
          {
            url: this.props.url,
            dataType: 'json',
            method: 'POST',
            data: JSON.stringify(
              {
                username: submitted_username,
                password: submitted_password
              })
          }).done(
            function(data, status, xhr) {
              console.log("done:  " + data);
              console.log("Request was successful");
              console.log("Response data: " + JSON.stringify(data) +
                " Status: " + status + " xhr: "+ xhr);
              $.ajax(
              {
                url: "/api/service_locator/logout",
                method: 'POST'
              }).done(
                function () {
                  console.log("Logged out and back to login page!");
                  this.setState({ submitted : false });
                  console.log("submitted: " + this.state.submitted);
                  window.clearInterval(intervalID);
                  window.clearTimeout(timeoutID);
                  window.location.href = "/index.html"
                }.bind(this));
            }.bind(this)).fail(
            function(data, xhr, status, err) {
              this.setState({ submitted : false });
              this.setState({ errorMessages : 'the username and password you entered don\’t match'});
              console.log("submitted: " + this.state.submitted);
              console.log("Request failed! ERROR Section");
              console.log("errorMsg: " + this.state.errorMessages);
              console.log("fail data:  " + data);
              console.log("Response data: " + JSON.stringify(data) +
                " Status: " + status + " xhr: " + xhr);
              window.clearInterval(intervalID);
              window.clearTimeout(timeoutID);
            }.bind(this));
        var timeoutID = window.setTimeout( function () {
          this.setState({ errorMessages : 'unable to connect, check your connection' });
          jqxhr.abort();
          console.log("errorMessages: " + this.state.errorMessages);
          }.bind(this),1000);
        var intervalID = window.setInterval(function () {
          this.setState({logo_src : 'img/spire_loading_animation.gif'});
          console.log("image src after: " + this.state.logo_src);
        }.bind(this), 1000);
      },
      handleUsernameChange: function (e) {
        this.setState({username: e.target.value});
      },
      handlePasswordChange: function (e) {
        this.setState({password: e.target.value});
      },
    });
    ReactDOM.render(<LoginPage url="/api/service_locator/login" />,
      document.getElementById("container"));
});
