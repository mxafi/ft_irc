#include "Command.h"

namespace irc {

static std::vector<std::string> parseKick(std::string clients) {
    std::vector<std::string> clientNicksToKick;
    std::stringstream ss(clients);
    std::string token;
    while (std::getline(ss, token, ',')) {
        clientNicksToKick.push_back(token);
    }
    return clientNicksToKick;
}

void Command::actionKick(Client& client) {
    std::string reasonToKick = client.getNickname();
    //Check parameterrs
    // Less than 2 parameters: reply ERR_NEEDMOREPARAMS
    if (param_.size() < 2) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "KICK"));
        return;
    }
    if (param_.size() == 3) {
        //Case 1: they didnt write a reason.
        //Case 2: they write the reason to kick.
        if (param_.at(2).front() == ':') {
            if (param_.at(2).length() > 1) {
                reasonToKick = param_.at(2).substr(1);
            }
        }
    }

    // validate client and channel
    std::vector<std::string> clientChannels = client.getMyChannels();
    bool isMember = false;
    for (std::string clientChannel : clientChannels) {
        if (clientChannel == param_.at(0)) {
            isMember = true;
            break;
        }
    }
    if (isMember == false) {
        try {
            Channel& channel = allChannels_.at(param_.at(0));
            (void)channel;  // suppress warning about unused variable
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, client.getNickname(), param_.at(0)));
            return;
        } catch (std::out_of_range& e) {
            client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, param_.at(0)));
            return;
        }
    }
    // at this point we know the channel exists and the client is member

    Channel& channel = allChannels_.at(param_.at(0));
    if (channel.isOperator(client) == false) {
        client.appendToSendBuffer(RPL_ERR_CHANOPRIVSNEEDED_482(serverHostname_g, client.getNickname(), param_.at(0)));
        return;
    }

    // at this point we know that the person is an operator

    // Now we check if the clients they are trying to kick exist.
    std::vector<std::string> clientsToKick = parseKick(param_.at(1));
    for (const std::string& clientNickToKick : clientsToKick) {
        try {
            (void)findClientByNicknameOrThrow(clientNickToKick);
        } catch (std::out_of_range& e) {
            client.appendToSendBuffer(RPL_ERR_NOSUCHNICK_401(serverHostname_g, clientNickToKick));
            return;
        }
        // At this point we know the client nick to be kicked exists.
        Client& clientToKick = findClientByNicknameOrThrow(clientNickToKick);
        // Here we check if he belongs to that channel.
        clientChannels = clientToKick.getMyChannels();
        isMember = false;
        for (std::string clientChannel : clientChannels) {
            if (clientChannel == param_.at(0)) {
                isMember = true;
                break;
            }
        }
        if (isMember == false) {
            client.appendToSendBuffer(RPL_ERR_USERNOTINCHANNEL_441(serverHostname_g, clientNickToKick, param_.at(0)));
            return;
        }
        // supposed to send: :nick!user@host KICK #channel nicktokick :reason
        channel.sendMessageToMembers(COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "KICK",
                                                 channel.getName() + " " + clientToKick.getNickname() + " :" + reasonToKick));
        channel.partMember(clientToKick);
    }
}

}  // namespace irc
